#include "process_queries.h"

#include <algorithm>
#include <execution>

std::vector<std::vector<Document>> ProcessQueries(
    const SearchServer& search_server,
    const std::vector<std::string>& queries){
    
    std::vector<std::vector<Document>> documents(queries.size());
    
    std::transform(std::execution::par ,queries.begin(), queries.end(), documents.begin(),
                  [&search_server](const std::string& query)
                   {
                       return search_server.FindTopDocuments(query);
                   });
    
    return documents;
}

std::vector<Document> ProcessQueriesJoined(
    const SearchServer& search_server,
    const std::vector<std::string>& queries){
    
    std::vector<std::vector<Document>> documents = std::move(ProcessQueries(search_server, queries));
    std::vector<Document> out;
    
    for(const auto& query : documents){
        out.insert(out.end(), query.begin(), query.end());
    }
    return out;
}