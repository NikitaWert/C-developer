#include "request_queue.h"

    std::vector<Document> RequestQueue::AddFindRequest(const  std::string& raw_query, DocumentStatus status) {
        auto doc = ser_.FindTopDocuments(raw_query, status);
        reTime(doc.size(), raw_query);
        return doc;
    }

    std::vector<Document> RequestQueue::AddFindRequest(const  std::string& raw_query) {
        auto doc = ser_.FindTopDocuments(raw_query);
        reTime(doc.size(), raw_query);
        return doc;
    }

    void RequestQueue::reTime(size_t size_doc, const  std::string& raw_query) {
        time_run_++;
        if (time_run_ == sec_in_day_) time_run_ = 0;
        if (!requests_.empty() && requests_.front().time == time_run_) requests_.pop_front();
        if (size_doc < 1) requests_.push_back({ requests_.size() + 1, raw_query });
    }