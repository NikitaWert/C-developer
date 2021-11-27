#include "transport_catalogue.h"
#include "json.h"
#include "json_reader.h"
#include "domain.h"
#include "serialization.h"

#include <string>
#include <iostream>
#include <vector>

using namespace std;

int ProcessRequests() {

	json::Document input_doc = json::Load(std::cin);

	TransportCatalogue transport_class;

	ForSerialization ser = input_json::GetSettingsForSerialization(input_doc.GetRoot().AsDict().at("serialization_settings").AsDict());

	ifstream file(ser.path_for_file, ios::binary);

    InformationForCatalog queres = Deserialize(file);
	std::vector<std::string> index_stops;

	for (auto& new_bus_stop : queres.set_bus_stop) {
		index_stops.push_back(new_bus_stop.name_bus_stop_);
		transport_class.AddBusStop(move(new_bus_stop));
	}

	for (auto& new_dist : queres.distance_) {
		transport_class.AddDistanceBetweenStops(move(new_dist));
	}

	for (auto& new_bus : queres.set_bus) {
		transport_class.AddBus(move(new_bus), index_stops);
	}

	vector<QuerysToOut> queres_out = input_json::ReadingQueries(input_doc.GetRoot().AsDict().at("stat_requests").AsArray());

	json::Array to_json;

	for (const auto& query : queres_out) {

		if (query.type == "Bus") {
			to_json.push_back(
				move(output_json::QueryBusToDict(move(transport_class.FindBus(query.name)), query.id)));
		}
		else if (query.type == "Stop") {
			to_json.push_back(
				move(output_json::QueryBusStopToDict(move(transport_class.FindBusStop(query.name)), query.id)));
		}

	}

	json::Document to_out = json::Document(json::Node(to_json));

	json::Print(to_out, std::cout);

	return 0;
}