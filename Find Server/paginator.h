#pragma once

#include <vector>
#include <algorithm>
#include <iostream>

template <typename Iterator>
class IteratorRange {
public:
    IteratorRange(Iterator begin, Iterator end)
        : first_(begin)
        , last_(end)
        , size_(distance(first_, last_)) {
    }

    inline Iterator begin() const noexcept{
        return first_;
    }

    inline Iterator end() const noexcept {
        return last_;
    }

    inline size_t size() const noexcept {
        return size_;
    }

private:
    Iterator first_, last_;
    size_t size_;
};

template <typename Iterator>
class Paginator {
public:
    Paginator(Iterator begin, Iterator end, size_t page_size) {
        for (size_t left = std::distance(begin, end); left > 0;) {
            const size_t current_page_size = std::min(page_size, left);
            const Iterator current_page_end = std::next(begin, current_page_size);
            pages_.push_back({ begin, current_page_end });
            
            left -= current_page_size;
            begin = current_page_end;
        }
    }

    inline auto begin() const noexcept{
        return pages_.begin();
    }

    inline auto end() const noexcept{
        return pages_.end();
    }

    inline size_t size() const noexcept{
        return pages_.size();
    }

private:
    std::vector<IteratorRange<Iterator>> pages_;
};

template <typename Container>
auto Paginate(const Container& c, size_t page_size) {
    return Paginator(begin(c), end(c), page_size);
}

template <typename Iterator>
std::ostream& operator<<(std::ostream& out, const IteratorRange<Iterator>& range) {
    for (Iterator it = range.begin(); it != range.end(); ++it) {
        out << *it;
    }
    return out;
}