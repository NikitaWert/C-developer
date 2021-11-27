#pragma once

#include "transport_catalogue.h"
#include "domain.h"
#include "geo.h"
#include "map_renderer.h"
#include "svg.h"
#include "json_builder.h"
#include "router.h"
#include "transport_router.h"

#include <string>
#include <iostream>
#include <set>
#include <vector>
#include <utility>
#include <sstream>
#include <optional>
#include <filesystem>
#include <unordered_map>

namespace transport_catalogue {

	namespace input_json {

		using Path = std::filesystem::path;

		detail::Bus ParseBus(const json::Dict& dic);

		detail::BusStop ParseBusStop(const json::Dict& dic);

		void ParseDistance(const json::Dict& dic, const std::string& from, InformationForCatalog& q);

		InformationForCatalog JsonReader(const json::Array& doc);

		std::vector<QuerysToOut> ReadingQueries(const json::Array& doc);

		renderer_for_set::RenderSettings GetRenderSettings(const json::Dict& doc);

		svg::Color GetColor(const json::Array& arr);

		RoutingSettings GetRoutingSettings(const json::Dict& dict);

		ForSerialization GetSettingsForSerialization(const json::Dict& dict);
	}

	namespace output_json {

		json::Node QueryBusToDict(const detail::BusRoute& bus, int id);

		json::Node QueryBusStopToDict(const detail::BusOnTheStop& out_stop, int id);

		json::Node SvgToJson(const svg::Document& doc, int id);

		json::Node RouteToJson(std::optional<transport_router::detail::BuildRoute> route, int id);
	}

}