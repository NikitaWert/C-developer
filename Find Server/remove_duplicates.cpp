#include"remove_duplicates.h"

#include<set>
#include<vector>

void RemoveDuplicates(SearchServer& search_server) {

	using namespace std;

	set<set<string>> for_duplicates;
	vector<int> duplicates_documents_id;

	for (auto begin = search_server.begin(); begin != search_server.end(); begin++) {
		const auto id = *begin;
		const auto& in = search_server.GetIdsWords(id);
		if (for_duplicates.find(in) != for_duplicates.end()) duplicates_documents_id.push_back(id);
		else for_duplicates.insert(in);
	}

	for (const auto& id : duplicates_documents_id) {
		search_server.RemoveDocument(id);
		cout << "Found duplicate document id " << id << endl;
	}
}