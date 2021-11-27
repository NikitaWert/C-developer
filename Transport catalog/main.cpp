#include "json.h"
#include "json_reader.h"
#include "domain.h"
#include "serialization.h"
#include "map_renderer.h"
#include "request_handler.h"
#include "transport_catalogue.pb.h"

#include <fstream>
#include <iostream>
#include <string_view>
#include <filesystem>
#include <vector>

using namespace std::literals;

void PrintUsage(std::ostream& stream = std::cerr) {
	stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[]) {
	if (argc != 2) {
		PrintUsage();
		return 1;
	}

	const std::string_view mode(argv[1]);

	if (mode == "make_base"sv) {

		TransportCatalogue transport_class;
		json::Document input_doc = json::Load(std::cin);

		InformationForCatalog queres = input_json::JsonReader(input_doc.GetRoot().AsDict().at("base_requests").AsArray());
		renderer_for_set::RenderSettings render_set(std::move(input_json::GetRenderSettings(input_doc.GetRoot().AsDict().at("render_settings").AsDict())));
		queres.routing_settings = input_json::GetRoutingSettings(input_doc.GetRoot().AsDict().at("routing_settings").AsDict());
		ForSerialization serial = input_json::GetSettingsForSerialization(input_doc.GetRoot().AsDict().at("serialization_settings").AsDict());

		for (auto& new_bus_stop : queres.set_bus_stop) {
			transport_class.AddBusStop(new_bus_stop);
		}

		for (auto& new_dist : queres.distance_) {
			transport_class.AddDistanceBetweenStops(new_dist);
		}

		for (auto& new_bus : queres.set_bus) {
			transport_class.AddBus(new_bus);
		}

		transport_router::InitGraph init(transport_class, queres.set_bus_stop.size() * 2);
		init.InfoForGraph(queres);

		std::ofstream file(serial.path_for_file, std::ios::binary);

		Serialization(file, queres, render_set, init);

	}
	else if (mode == "process_requests"sv) {

		json::Document input_doc = json::Load(std::cin);

		TransportCatalogue transport_class;

		ForSerialization ser = input_json::GetSettingsForSerialization(input_doc.GetRoot().AsDict().at("serialization_settings").AsDict());

		std::ifstream file(ser.path_for_file, std::ios::binary);

		transport_set::Transport_set trans_list;
		trans_list.ParseFromIstream(&file);

		InformationForCatalog queres(std::move(DeserializeCatalog(trans_list)));
		renderer_for_set::RenderSettings render_set(std::move(DeserializeMap(trans_list)));
		InformationForCatalog queres_for_graph{ queres.set_bus, queres.set_bus_stop, {}, queres.routing_settings, {} };

		for (auto& new_bus_stop : queres.set_bus_stop) {
			transport_class.AddBusStop(std::move(new_bus_stop));
		}

		for (auto& new_dist : queres.distance_) {
			transport_class.AddDistanceBetweenStops(std::move(new_dist));
		}

		for (auto& new_bus : queres.set_bus) {
			transport_class.AddBus(std::move(new_bus));
		}

		transport_router::InitGraph init(transport_class, std::move(DeserializeGraph(trans_list)));
		transport_router::TransportRouter router(init);

		renderer_for_set::MapRenderer ren(std::move(render_set));
		const handler_for_set::RequestHandler hend(transport_class, ren);

		std::vector<QuerysToOut> queres_out = input_json::ReadingQueries(input_doc.GetRoot().AsDict().at("stat_requests").AsArray());

		json::Array to_json;
		auto doc = std::move(output_json::SvgToJson(hend.RenderMap(queres_for_graph), 1));

		for (const auto& query : queres_out) {

			if (query.type == "Bus") {
				to_json.push_back(
					std::move(output_json::QueryBusToDict(std::move(transport_class.FindBus(query.name)), query.id)));
			}
			else if (query.type == "Stop") {
				to_json.push_back(
					std::move(output_json::QueryBusStopToDict(std::move(transport_class.FindBusStop(query.name)), query.id)));
			}
			else if (query.type == "Map") {

				to_json.push_back(std::move(json::Builder{}.StartDict()
					.Key("request_id"s).Value(query.id).Key("map")
					.Value(doc.AsDict().at("map").AsString()).EndDict().Build()));
			}
			else if (query.type == "Route") {
				to_json.push_back(
					output_json::RouteToJson(std::move(router.GetItemsRoute(query.from, query.to)), query.id));
			}
		}

		json::Document to_out = json::Document(json::Node(to_json));

		json::Print(to_out, std::cout);

	}
	else {
		PrintUsage();
		return 1;
	}
}