#pragma once
#include "document.h"
#include <map>
#include <set>
#include <string>
#include <iostream>
#include <vector>
#include <algorithm>

const int MAX_RESULT_DOCUMENT_COUNT = 5;

class SearchServer {
public:

    SearchServer() = default;

    SearchServer(const std::string& stop_words_text) {
        SetStopWords(stop_words_text);
    }

    template <typename StringContainer>
    explicit SearchServer(const StringContainer& stop_words);

    void SetStopWords(const std::string& text);
    void AddDocument(int document_id, const std::string& document, DocumentStatus status, const std::vector<int>& ratings);

    std::vector<Document> FindTopDocuments(const std::string& raw_query, DocumentStatus status = DocumentStatus::ACTUAL) const;

    template<typename KeyMapper>
    std::vector<Document> FindTopDocuments(const std::string& raw_query, KeyMapper key_mapper) const;

    inline int GetDocumentCount() const noexcept {
        return documents_.size();
    }

    inline bool IsContainWord(const std::string& word) const noexcept {
        return word_to_document_freqs_.count(word);
    }

    inline bool IsContainWordId(const std::string& word, int document_id) const noexcept{
        return word_to_document_freqs_.at(word).count(document_id);
    }

    std::tuple<std::vector<std::string>, DocumentStatus> MatchDocument(const std::string& raw_query, int document_id) const;

    inline int GetDocumentId(int index) const {
        return index_.at(index);
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
    std::vector<int> index_;

    static int ComputeAverageRating(const std::vector<int>& ratings);

    bool IsValid(const std::string& words) const;

    void ValidWord(const std::string& word) const;

    void ValidParseWords(Query q) const;

    inline bool IsStopWord(const std::string& word) const {
        return stop_words_.count(word) > 0;
    }

    std::vector<std::string> SplitIntoWordsNoStop(const std::string& text) const;

    QueryWord ParseQueryWord(std::string text) const;

    Query ParseQuery(const std::string& text) const;

    double ComputeWordInverseDocumentFreq(const std::string& word) const;

    template<typename KeyMapper>
    std::vector<Document> FindAllDocuments(const Query& query, KeyMapper key_mapper) const;
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

    SearchServer{ text };
}

template<typename KeyMapper>
std::vector<Document> SearchServer::FindTopDocuments(const std::string& raw_query, KeyMapper key_mapper) const {

    if (!IsValid(raw_query)) throw std::invalid_argument("Недопустимые знаки в запросе");

    const Query query = ParseQuery(raw_query);
    ValidParseWords(query);
    auto matched_documents = FindAllDocuments(query, key_mapper);

    std::sort(matched_documents.begin(), matched_documents.end(),
        [](const Document& lhs, const Document& rhs) {
            return ((abs(lhs.relevance - rhs.relevance)) < 1e-6) ?
                lhs.rating > rhs.rating : lhs.relevance > rhs.relevance;
        });
    if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
        matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
    }
    return matched_documents;

    return {};
}

template<typename KeyMapper>
std::vector<Document> SearchServer::FindAllDocuments(const Query& query, KeyMapper key_mapper) const {
    std::map<int, double> document_to_relevance;
    for (const std::string& word : query.plus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);
        for (const auto& [document_id, term_freq] : word_to_document_freqs_.at(word)) {
            if (key_mapper(document_id, documents_.at(document_id).status, documents_.at(document_id).rating)) {
                document_to_relevance[document_id] += term_freq * inverse_document_freq;
            }
        }
    }

    for (const std::string& word : query.minus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        for (const auto& [document_id, _] : word_to_document_freqs_.at(word)) {
            document_to_relevance.erase(document_id);
        }
    }

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