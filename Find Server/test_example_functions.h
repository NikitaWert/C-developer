#pragma once

#include"search_server.h"
#include"document.h"

#include<vector>

void AddDocument(SearchServer& search_server, int id, const std::string& words, DocumentStatus status, const std::vector<int>& ratings);