#include"test_example_functions.h"

void AddDocument(SearchServer& search_server,
	int id, const std::string& words, DocumentStatus status, const std::vector<int>& ratings) {
	search_server.AddDocument(id, words, status, ratings);
}