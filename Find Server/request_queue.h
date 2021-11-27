#pragma once

#include "search_server.h"
#include "document.h"

#include <string>
#include <deque>

class RequestQueue {
public:
    explicit RequestQueue(const SearchServer& search_server) : ser_(search_server) {}
   
    template <typename DocumentPredicate>
    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate);

    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentStatus status);

    std::vector<Document> AddFindRequest(const std::string& raw_query);

    inline int GetNoResultRequests() const noexcept{
        return requests_.size();
    }

private:
    struct QueryResult {
        size_t time;
        std::string raw;
    };

    std::deque<QueryResult> requests_;
    const static int sec_in_day_ = 1440;
    int time_run_ = 0;
    const SearchServer& ser_;

    void reTime(size_t size_doc, const std::string& raw_query);
};

template <typename DocumentPredicate>
std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate) {
    auto doc = ser_.FindTopDocuments(raw_query, document_predicate);
    reTime(doc.size(), raw_query);
    return doc;
}