#include "json_reader.h"

using namespace transport_catalogue;

using namespace detail;

detail::Bus input_json::ParseBus(const json::Dict& dic) {

	Bus new_bus;

	new_bus.number_bus_ = dic.at("name").AsString();
	new_bus.circl = dic.at("is_roundtrip").AsBool();

	for (const auto& str : dic.at("stops").AsArray()) {
		new_bus.bus_stop_.push_back(str.AsString());
	}

	return new_bus;
}

BusStop input_json::ParseBusStop(const json::Dict& dic) {

	BusStop new_bus_stop;

	new_bus_stop.name_bus_stop_ = dic.at("name").AsString();
	new_bus_stop.cordinat_bus_stop_.lat = dic.at("latitude").AsDouble();
	new_bus_stop.cordinat_bus_stop_.lng = dic.at("longitude").AsDouble();

	return new_bus_stop;
}

void input_json::ParseDistance(const json::Dict& dic, const std::string& from, InformationForCatalog& q) {

	if (dic.empty()) return;

	for (const auto& [key, d] : dic) {

		DistanceToStop dist;

		dist.name_stop_from_ = from;
		dist.name_stop_to_ = key;
		dist.dist_in_meters_ = d.AsInt();

		q.distance_.push_back(std::move(dist));
	}

}

InformationForCatalog input_json::JsonReader(const json::Array& doc) {

	InformationForCatalog out;

	for (const auto& item_arr : doc) {

		if (item_arr.AsDict().at("type").AsString() == "Stop") {

			out.set_bus_stop.push_back(ParseBusStop(item_arr.AsDict()));
			if (item_arr.AsDict().count("road_distances")) {
				ParseDistance(item_arr.AsDict().at("road_distances").AsDict(),
					item_arr.AsDict().at("name").AsString(),
					out);
			}

			out.index_stops[out.set_bus_stop.back().name_bus_stop_] = out.set_bus_stop.size() - 1;
		}
	}

	for (const auto& item_arr : doc) {

		if (item_arr.AsDict().at("type").AsString() == "Bus") {
			out.set_bus.push_back(ParseBus(item_arr.AsDict()));
		}
	}

	return out;
}

std::vector<QuerysToOut> input_json::ReadingQueries(const json::Array& doc) {

	std::vector<QuerysToOut> out;

	for (const auto& item_dic : doc) {

		QuerysToOut new_out;

		new_out.id = item_dic.AsDict().at("id").AsInt();
		if (item_dic.AsDict().at("type").AsString() == "Map") {
			new_out.type = item_dic.AsDict().at("type").AsString();
		}
		else if (item_dic.AsDict().at("type").AsString() == "Route") {
			new_out.from = item_dic.AsDict().at("from").AsString();
			new_out.to = item_dic.AsDict().at("to").AsString();
			new_out.type = item_dic.AsDict().at("type").AsString();
		}
		else {
			new_out.name = item_dic.AsDict().at("name").AsString();
			new_out.type = item_dic.AsDict().at("type").AsString();
		}

		out.push_back(new_out);
	}

	return out;
}

json::Node output_json::QueryBusToDict(const detail::BusRoute& bus, int id) {

	using namespace std::string_literals;

	json::Builder build;
	build.StartDict().Key("request_id"s).Value(id);

	if (!bus.find_bus) {
		return build.Key("error_message"s).Value("not found"s).EndDict().Build();
	}

	return build.Key("curvature"s).Value(bus.curvature)
		.Key("route_length"s).Value(static_cast<int>(bus.total_distance_in_meters))
		.Key("stop_count"s).Value(static_cast<int>(bus.count_bus_stop))
		.Key("unique_stop_count"s).Value(static_cast<int>(bus.unique_bus_stop)).EndDict().Build();
}

json::Node output_json::QueryBusStopToDict(const detail::BusOnTheStop& out_stop, int id) {

	using namespace std::string_literals;

	json::Dict new_dict;
	json::Builder build;

	build.StartDict().Key("request_id"s).Value(id);

	if (out_stop.name_stop.empty()) {
		return build.Key("error_message"s).Value("not found"s).EndDict().Build();
	}
	else if (out_stop.number_buses.empty()) {
		return build.Key("buses"s).Value(json::Array{}).EndDict().Build();
	}

	return build.Key("buses"s).Value(json::Array(out_stop.number_buses.begin(), out_stop.number_buses.end()))
		.EndDict().Build();
}

renderer_for_set::RenderSettings input_json::GetRenderSettings(const json::Dict& doc) {

	using namespace std::string_literals;

	renderer_for_set::RenderSettings ren;

	ren.width_ = doc.at("width"s).AsDouble();
	ren.height_ = doc.at("height"s).AsDouble();
	ren.padding_ = doc.at("padding"s).AsDouble();
	ren.line_width_ = doc.at("line_width"s).AsDouble();
	ren.stop_radius_ = doc.at("stop_radius"s).AsDouble();
	ren.bus_label_font_size_ = doc.at("bus_label_font_size"s).AsInt();
	ren.bus_label_offset_.x = doc.at("bus_label_offset"s).AsArray().at(0).AsDouble();
	ren.bus_label_offset_.y = doc.at("bus_label_offset"s).AsArray().at(1).AsDouble();
	ren.stop_label_font_size_ = doc.at("stop_label_font_size"s).AsInt();
	ren.stop_label_offset_.x = doc.at("stop_label_offset"s).AsArray().at(0).AsDouble();
	ren.stop_label_offset_.y = doc.at("stop_label_offset"s).AsArray().at(1).AsDouble();

	(!doc.at("underlayer_color"s).IsString()) ?
		ren.underlayer_color_ = GetColor(doc.at("underlayer_color"s).AsArray()) : ren.underlayer_color_ = doc.at("underlayer_color").AsString();

	ren.underlayer_width_ = doc.at("underlayer_width"s).AsDouble();

	for (const auto& color : doc.at("color_palette"s).AsArray()) {
		(color.IsString()) ?
			ren.palette_.push_back(color.AsString()) : ren.palette_.push_back(GetColor(color.AsArray()));
	}

	return ren;
}

svg::Color input_json::GetColor(const json::Array& arr) {

	uint8_t red = static_cast<uint8_t>(arr.at(0).AsInt());
	uint8_t green = static_cast<uint8_t>(arr.at(1).AsInt());
	uint8_t blue = static_cast<uint8_t>(arr.at(2).AsInt());

	return (arr.size() == 3) ? svg::Color(svg::Rgb(red, green, blue)) : svg::Color(svg::Rgba(red, green, blue, arr.at(3).AsDouble()));
}

json::Node output_json::SvgToJson(const svg::Document& doc, int id) {

	using namespace std::string_literals;

	std::stringstream ss;
	doc.Render(ss);

	return json::Builder{}.StartDict().Key("map"s).Value(ss.str())
		.Key("id"s).Value(id).EndDict().Build();
}

RoutingSettings input_json::GetRoutingSettings(const json::Dict & dict) {
	return RoutingSettings{ dict.at("bus_velocity").AsDouble(), dict.at("bus_wait_time").AsInt() };
}

json::Node output_json::RouteToJson(std::optional<transport_router::detail::BuildRoute> route, int id) {
	
	using namespace std::string_literals;

	json::Builder build;
	build.StartDict();

	if (!route.has_value()) {
		return build.Key("request_id"s).Value(id).Key("error_message"s).Value("not found"s)
			.EndDict().Build();
	}	

	build.Key("request_id"s).Value(id).Key("total_time").Value((*route).time)
		.Key("items"s).StartArray();

	for (auto& item : (*route).items) {
		if (item.type == "Wait"s) {
			build.StartDict().Key("stop_name"s).Value(std::move(item.name))
				.Key("time"s).Value(item.time)
				.Key("type"s).Value(std::move(item.type)).EndDict();
		}
		else {
			build.StartDict().Key("bus"s).Value(std::move(item.name))
				.Key("span_count"s).Value(item.span_count)
				.Key("time"s).Value(item.time)
				.Key("type"s).Value(std::move(item.type)).EndDict();
		}
	}

	 return build.EndArray().EndDict().Build();
}

ForSerialization input_json::GetSettingsForSerialization(const json::Dict& dict) {
	ForSerialization out;
	out.path_for_file = dict.at("file").AsString();
	
	return out;
}

