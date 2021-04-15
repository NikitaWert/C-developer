#include "search_server.h"
#include "string_processing.h"

using namespace std::string_literals;

    void SearchServer::SetStopWords(const std::string& text) {
        if (!IsValid(text)) throw std::invalid_argument("Недопустимые знаки"s);
        for (const std::string& word : SplitIntoWords(text)) {
            stop_words_.insert(word);
        }
    }

    void SearchServer::AddDocument(int document_id, const std::string& document, DocumentStatus status, const std::vector<int>& ratings) {


        if (document_id < 0) throw std::invalid_argument("Отрицательный id "s + std::to_string(document_id));
        if (!IsValid(document)) throw std::invalid_argument("Недопустимые знаки"s);
        if (documents_.count(document_id)) throw std::invalid_argument("Документ с таким id уже есть"s + "("s + std::to_string(document_id) + ")");

        index_.push_back(document_id);

        const std::vector<std::string> words = SplitIntoWordsNoStop(document);
        const double inv_word_count = 1.0 / words.size();
        for (const std::string& word : words) {
            word_to_document_freqs_[word][document_id] += inv_word_count;
        }
        documents_.emplace(document_id,
            DocumentData{
                ComputeAverageRating(ratings),
                status
            });
    }

    std::vector<Document> SearchServer::FindTopDocuments(const std::string& raw_query, DocumentStatus status) const {
        return FindTopDocuments(raw_query, [status](int document_id, DocumentStatus status_document, int rating) { return status_document == status; });
    }

    std::tuple<std::vector<std::string>, DocumentStatus> SearchServer::MatchDocument(const std::string& raw_query, int document_id) const {


        if (!IsValid(raw_query)) throw std::invalid_argument("Недопустимые знаки в запросе");

        const Query query = ParseQuery(raw_query);
        ValidParseWords(query);
        std::vector<std::string> matched_words;
        for (const std::string& word : query.plus_words) {
            if (IsContainWord(word) && IsContainWordId(word, document_id)) {
                matched_words.push_back(word);
            }
        }

        for (const std::string& word : query.minus_words) {
            if (IsContainWord(word) && IsContainWordId(word, document_id)) {
                matched_words.clear();
                break;
            }
        }
        return { matched_words, documents_.at(document_id).status };

        return {};
    }

    int SearchServer::ComputeAverageRating(const std::vector<int>& ratings) {
        int rating_sum = 0;
        for (const int rating : ratings) {
            rating_sum += rating;
        }
        return rating_sum / static_cast<int>(ratings.size());
    }

    bool SearchServer::IsValid(const std::string& words) const {
        return none_of(words.begin(), words.end(), [](char c) {
            return c >= '\0' && c < ' ';
            });
    }

    void SearchServer::ValidWord(const std::string& word)const {
        if (word == "") throw std::invalid_argument("Некорректное слово - " + word);
        if (word.length() > 1 && word[0] == '-') throw std::invalid_argument("Перед словом два минуса - " + word);
    }

    void SearchServer::ValidParseWords(Query q) const {

        for (const auto& word : q.plus_words) {
            ValidWord(word);
        }
        for (const auto& word : q.minus_words) {
            ValidWord(word);
        }
    }

    std::vector<std::string> SearchServer::SplitIntoWordsNoStop(const std::string& text) const {
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

    SearchServer::Query SearchServer::ParseQuery(const std::string& text) const {
        Query query;
        for (const std::string& word : SplitIntoWords(text)) {
            const QueryWord query_word = ParseQueryWord(word);

            if (query_word.is_stop) continue;

            if (query_word.is_minus) {
                query.minus_words.insert(query_word.data);
            }
            else {
                query.plus_words.insert(query_word.data);
            }
        }
        return query;
    }


    double SearchServer::ComputeWordInverseDocumentFreq(const std::string& word) const {
        return log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(word).size());
    }