#pragma once

#include "document.h"

#include <map>
#include <set>
#include <string>
#include <iostream>
#include <vector>
#include <algorithm>
#include <iterator>
#include <cmath>
#include <execution>
#include <string_view>
#include <mutex>

const int MAX_RESULT_DOCUMENT_COUNT = 5;

class SearchServer {
public:

    SearchServer() = default;

    SearchServer(const std::string_view stop_words_text) {
        SetStopWords(stop_words_text);
    }

    SearchServer(const std::string& stop_words_text) {
        SetStopWords(stop_words_text);
    }

    template <typename StringContainer>
    explicit SearchServer(const StringContainer& stop_words);

    void SetStopWords(const std::string_view text);

    void AddDocument(int document_id, const std::string_view document, DocumentStatus status, const std::vector<int>& ratings);

    std::vector<Document> FindTopDocuments(const std::string_view raw_query, DocumentStatus status = DocumentStatus::ACTUAL) const;

    template<typename KeyMapper, class ExecutionPolicy>
    std::vector<Document> FindTopDocuments(ExecutionPolicy&& policy, const std::string_view raw_query, KeyMapper key_mapper) const;

    template<typename KeyMapper>
    std::vector<Document> FindTopDocuments(const std::string_view raw_query, KeyMapper key_mapper) const {
        return FindTopDocuments(std::execution::seq, raw_query, key_mapper);
    }

    template<class ExecutionPolicy >
    std::vector<Document> FindTopDocuments(ExecutionPolicy&& policy, const std::string_view raw_query, DocumentStatus status = DocumentStatus::ACTUAL) const {
        return FindTopDocuments(policy, raw_query, [status](int document_id, DocumentStatus status_document, int rating) { return status_document == status; });
    }

    inline int GetDocumentCount() const noexcept {
        return documents_.size();
    }

    std::tuple<std::vector<std::string_view>, DocumentStatus> MatchDocument(const std::string_view raw_query, int document_id) const;

    template<class ExecutionPolicy>
    std::tuple<std::vector<std::string_view>, DocumentStatus> MatchDocument(ExecutionPolicy&& policy, const std::string_view raw_query, int document_id) const;

    const std::map<std::string_view, double>& GetWordFrequencies(int document_id) const;

    void RemoveDocument(const std::execution::sequenced_policy&, int document_id) {
        RemoveDocument(document_id);
    }

    void RemoveDocument(int document_id);
    
    void RemoveDocument(const std::execution::parallel_policy&, int document_id);

    inline std::set<int>::const_iterator begin() noexcept {
        return document_id_.cbegin();
    }

    inline std::set<int>::const_iterator end() noexcept {
        return document_id_.cend();
    }

    inline const std::set<std::string>& GetIdsWords(int document_id) const {
        return index_words_.at(document_id);
    }

private:

    struct DocumentData {
        int rating;
        DocumentStatus status;
    };

    struct QueryWord {
        std::string data;
        bool is_minus;
        bool is_stop;
    };

    struct Query {
        std::set<std::string> plus_words;
        std::set<std::string> minus_words;
    };

    std::set<std::string> stop_words_;
    std::map<std::string, std::map<int, double>> word_to_document_freqs_;
    std::map<int, DocumentData> documents_;
    std::map<int, std::set<std::string>> index_words_;
    std::set<int> document_id_;

    static int ComputeAverageRating(const std::vector<int>& ratings);

    bool IsValid(const std::string_view words) const;

    void ValidWord(const std::string& word) const;

    void ValidParseWords(Query q) const;

    inline bool IsStopWord(const std::string& word) const {
        return stop_words_.count(word) > 0;
    }

    inline bool IsContainWord(const std::string& word) const noexcept {
        return word_to_document_freqs_.count(word);
    }

    inline bool IsContainWordId(const std::string& word, int document_id) const noexcept {
        return word_to_document_freqs_.at(word).count(document_id);
    }

    std::vector<std::string> SplitIntoWordsNoStop(const std::string_view text) const;

    QueryWord ParseQueryWord(std::string text) const;

    Query ParseQuery(const std::string_view text) const;

    double ComputeWordInverseDocumentFreq(const std::string& word) const;

    template<typename KeyMapper, class ExecutionPolicy>
    std::vector<Document> FindAllDocuments(ExecutionPolicy&& policy, const Query& query, KeyMapper key_mapper) const;
};

template <typename StringContainer>
SearchServer::SearchServer(const StringContainer& stop_words) {

    std::string text;
    bool first = true;
    for (const auto& word : stop_words) {
        if (first) {
            text += word;
            first = false;
            continue;
        }
        text += " " + word;
    }

    SetStopWords(text);
}

template<typename KeyMapper, class ExecutionPolicy>
std::vector<Document> SearchServer::FindTopDocuments(ExecutionPolicy&& policy, const std::string_view raw_query, KeyMapper key_mapper) const {

    if (!IsValid(raw_query)) throw std::invalid_argument("Недопустимые знаки в запросе");

    const Query query = ParseQuery(raw_query);
    ValidParseWords(query);
    auto matched_documents = FindAllDocuments(policy, query, key_mapper);

    std::sort(policy, matched_documents.begin(), matched_documents.end(),
        [](const Document& lhs, const Document& rhs) {
            return ((std::abs(lhs.relevance - rhs.relevance)) < 1e-6) ?
                lhs.rating > rhs.rating : lhs.relevance > rhs.relevance;
        });

    if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
        matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
    }

    return matched_documents;
}

template<typename KeyMapper, class ExecutionPolicy>
std::vector<Document> SearchServer::FindAllDocuments(ExecutionPolicy&& policy, const Query& query, KeyMapper key_mapper) const {
    std::map<int, double> document_to_relevance;
    std::mutex stop_insert_map;

    std::for_each(policy, query.plus_words.begin(), query.plus_words.end(),
        [&](const std::string& word){
            if (word_to_document_freqs_.count(word) != 0) {

                const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);
                for (const auto& [document_id, term_freq] : word_to_document_freqs_.at(word)) {
                    if (key_mapper(document_id, documents_.at(document_id).status, documents_.at(document_id).rating)) {
                        std::lock_guard guard_map(stop_insert_map);
                        document_to_relevance[document_id] += term_freq * inverse_document_freq;
                    }
                }
            }
        });

    std::mutex stop_erase_map;
    std::for_each(policy, query.minus_words.begin(), query.minus_words.end(),
        [&](const std::string& word) {
            if (word_to_document_freqs_.count(word) != 0) {
                for (const auto& [document_id, _] : word_to_document_freqs_.at(word)) {
                    std::lock_guard guard_map(stop_erase_map);
                    document_to_relevance.erase(document_id);
                }
            }
        });

    std::vector<Document> matched_documents;
    for (const auto& [document_id, relevance] : document_to_relevance) {
        matched_documents.push_back({
            document_id,
            relevance,
            documents_.at(document_id).rating
            });
    }

    return matched_documents;
}

template<class ExecutionPolicy>
std::tuple<std::vector<std::string_view>, DocumentStatus> SearchServer::MatchDocument(ExecutionPolicy&& policy, const std::string_view raw_query, int document_id) const {
    if (!IsValid(raw_query)) throw std::invalid_argument("Недопустимые знаки в запросе");

    const Query query = ParseQuery(raw_query);
    ValidParseWords(query);
    std::vector<std::string_view> matched_words;

    std::for_each(policy, query.plus_words.begin(), query.plus_words.end(),
        [&](const std::string& word)mutable {
            if (this->IsContainWord(word) && this->IsContainWordId(word, document_id)) {
                matched_words.push_back(std::string_view{ *index_words_.at(document_id).find(word) });
            }
        });

    for_each(policy, query.minus_words.begin(), query.minus_words.end(),
        [&](const std::string& word) mutable {
            if (this->IsContainWord(word) && this->IsContainWordId(word, document_id)) {
                matched_words.clear();
            }
        });

    return { matched_words, documents_.at(document_id).status };
}