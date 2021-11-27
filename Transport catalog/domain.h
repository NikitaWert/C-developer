#pragma once

#include "transport_catalogue.h"

#include <vector>
#include <set>

using namespace transport_catalogue;

struct RoutingSettings {
	double bus_velocity;
	int bus_wait_time;
};

struct InformationForCatalog {
	std::vector<detail::Bus> set_bus;
	std::vector<detail::BusStop> set_bus_stop;
	std::vector<detail::DistanceToStop> distance_;
	RoutingSettings routing_settings;
	std::unordered_map<std::string, size_t, std::hash<std::string>> index_stops;
};

struct QuerysToOut {
	int id;
	std::string type;
	std::string name;
	std::string from;
	std::string to;
};

struct ForSerialization {
	std::string path_for_file;
};