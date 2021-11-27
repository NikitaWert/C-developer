#include "search_server.h"
#include "string_processing.h"

#include <future>
#include <execution>

using namespace std::string_literals;

    void SearchServer::SetStopWords(std::string_view text) {
        if (!IsValid(text)) throw std::invalid_argument("Недопустимые знаки"s);
        for (const std::string& word : SplitIntoWords(text)) {
            stop_words_.insert(word);
        }
    }

    void SearchServer::AddDocument(int document_id, std::string_view document, DocumentStatus status, const std::vector<int>& ratings) {

        if (document_id < 0) throw std::invalid_argument("Отрицательный id "s + std::to_string(document_id));
        if (!IsValid(document)) throw std::invalid_argument("Недопустимые знаки"s);
        if (documents_.count(document_id)) throw std::invalid_argument("Документ с таким id уже есть"s + "("s + std::to_string(document_id) + ")");

        document_id_.insert(document_id);
        const std::vector<std::string> words = SplitIntoWordsNoStop(document);
        const double inv_word_count = 1.0 / words.size();

        for (const std::string& word : words) {
            word_to_document_freqs_[word][document_id] += inv_word_count;
            index_words_[document_id].insert(word);
        }

        documents_.emplace(document_id,
            DocumentData{
                ComputeAverageRating(ratings),
                status
            });        
    }

    std::vector<Document> SearchServer::FindTopDocuments(std::string_view raw_query, DocumentStatus status) const {
        return FindTopDocuments(std::execution::seq, raw_query, [status](int document_id, DocumentStatus status_document, int rating) { return status_document == status; });
    }

    std::tuple<std::vector<std::string_view>, DocumentStatus> SearchServer::MatchDocument(const std::string_view raw_query, int document_id) const {

        if (!IsValid(raw_query)) throw std::invalid_argument("Недопустимые знаки в запросе");

        const Query query = ParseQuery(raw_query);
        ValidParseWords(query);
        std::vector<std::string_view> matched_words;

        for (const std::string& word : query.plus_words) {
            if (IsContainWord(word) && IsContainWordId(word, document_id)) {
                matched_words.push_back(std::string_view { *index_words_.at(document_id).find(word) });
            }
        }

        for (const std::string& word : query.minus_words) {
            if (IsContainWord(word) && IsContainWordId(word, document_id)) {
                matched_words.clear();
                break;
            }
        }
        return { matched_words, documents_.at(document_id).status };
    }

    int SearchServer::ComputeAverageRating(const std::vector<int>& ratings) {
        int rating_sum = 0;
        for (const int rating : ratings) {
            rating_sum += rating;
        }
        return rating_sum / static_cast<int>(ratings.size());
    }

    bool SearchServer::IsValid(std::string_view words) const {
        return std::none_of(words.begin(), words.end(), [](char c) {
            return c >= '\0' && c < ' ';
            });
    }

    void SearchServer::ValidWord(const std::string& word)const {
        if (word == "") throw std::invalid_argument("Некорректное слово - " + word);
        if (word.length() > 1 && word[0] == '-') throw std::invalid_argument("Перед словом два минуса - " + word);
    }

    void SearchServer::ValidParseWords(Query q) const {

        auto valid_plus_words = std::async([&]() { 
            for (const auto& word : q.plus_words) {
                ValidWord(word);
            }
        });

        for (const auto& word : q.minus_words) {
            ValidWord(word);
        }

        valid_plus_words.get();
    }

    std::vector<std::string> SearchServer::SplitIntoWordsNoStop(std::string_view text) const {
        std::vector<std::string> words;
        for (const std::string& word : SplitIntoWords(text)) {
            if (!IsStopWord(word)) {
                words.push_back(word);
            }
        }
        return words;
    }

    SearchServer::QueryWord SearchServer::ParseQueryWord(std::string text) const {
        bool is_minus = false;
        if (!text.empty()) {
            if (text[0] == '-') {
                is_minus = true;
                text = text.substr(1);
            }
        }
        return {
            text,
            is_minus,
            IsStopWord(text)
        };
    }

    SearchServer::Query SearchServer::ParseQuery(std::string_view text) const {
        Query query;

        auto split_word = SplitIntoWords(text);
        std::mutex lock_minus;
        std::mutex lock_plus;

        std::for_each(split_word.begin(), split_word.end(),
            [&](const std::string word) {
                const QueryWord query_word = ParseQueryWord(word);

                if (!query_word.is_stop) {
                    if (query_word.is_minus) {
                        std::lock_guard guard_minus(lock_minus);
                        query.minus_words.insert(query_word.data);
                    }
                    else {
                        std::lock_guard guard_plus(lock_plus);
                        query.plus_words.insert(query_word.data);
                    }
                }
            }
        );

        return query;
    }

    const std::map<std::string_view, double>& SearchServer::GetWordFrequencies(int document_id) const{
       
        static std::map<std::string_view, double> out;
        out.clear();

        if (index_words_.empty() || !index_words_.count(document_id)) return out;        

        for (const auto& word : index_words_.at(document_id)) {
            out[word] = word_to_document_freqs_.at(word).at(document_id);
        }       

        return out;
    } 

    void SearchServer::RemoveDocument(int document_id) {

        if (!index_words_.count(document_id)) return;

        for (const auto& word : index_words_.at(document_id)) {
            word_to_document_freqs_[word].erase(document_id);
        }

        document_id_.erase(document_id);
        documents_.erase(document_id);
        index_words_.erase(document_id);
    }

    void SearchServer::RemoveDocument(const std::execution::parallel_policy&, int document_id) {

        if (!index_words_.count(document_id)) return;

        std::for_each(std::execution::par, index_words_.at(document_id).begin(), index_words_.at(document_id).end(),
            [&](const std::string& word) {
                word_to_document_freqs_.at(word).erase(document_id);
            });

        document_id_.erase(document_id);
        documents_.erase(document_id);
        index_words_.erase(document_id);
    }
    
    double SearchServer::ComputeWordInverseDocumentFreq(const std::string& word) const {
        return log(GetDocumentCount() * 1.0 / static_cast<double>(word_to_document_freqs_.at(word).size()));
    }