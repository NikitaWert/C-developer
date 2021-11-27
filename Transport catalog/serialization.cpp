#include "serialization.h"
#include "svg.h"

#include <string>
#include <ostream>
#include <variant>
#include <vector>

void SerializationCatalog(const InformationForCatalog& query,
	transport_set::Transport_set& trans_list) {

	for (const auto& bus : query.set_bus) {

		transport_set::Bus pro_bus;

		std::string str = bus.number_bus_;

		pro_bus.set_number_bus(std::move(str));
		pro_bus.set_circl(bus.circl);

		for (const auto& stop_name : bus.bus_stop_) {
			pro_bus.add_index_stops(query.index_stops.at(stop_name));
		}

		*trans_list.add_buses() = std::move(pro_bus);
	}

	for (const auto& stop : query.set_bus_stop) {

		transport_set::Stop pro_stop;

		pro_stop.set_name_bus_stop(stop.name_bus_stop_);

		pro_stop.mutable_cordinates()->set_lat(stop.cordinat_bus_stop_.lat);
		pro_stop.mutable_cordinates()->set_lng(stop.cordinat_bus_stop_.lng);

		*trans_list.add_stops() = std::move(pro_stop);
	}

	for (const auto& dist : query.distance_) {

		transport_set::DistanceBetweenStops new_dist;

		new_dist.set_distance(dist.dist_in_meters_);
		new_dist.set_stop_from(query.index_stops.at(dist.name_stop_from_));
		new_dist.set_stop_to(query.index_stops.at(dist.name_stop_to_));

		*trans_list.add_distance() = std::move(new_dist);
	}
}

void SerializationMap(const renderer_for_set::RenderSettings& render_set,
	transport_set::Render_settings& render) {

	render.set_width(render_set.width_);
	render.set_height(render_set.height_);
	render.set_padding(render_set.padding_);
	render.set_line_width(render_set.line_width_);
	render.set_stop_radius(render_set.stop_radius_);
	render.set_bus_lable_font_size(render_set.bus_label_font_size_);
	render.mutable_bus_lable_offset()->set_x(render_set.bus_label_offset_.x);
	render.mutable_bus_lable_offset()->set_y(render_set.bus_label_offset_.y);
	render.set_stop_lable_font_size(render_set.stop_label_font_size_);
	render.mutable_stop_label_offset()->set_x(render_set.stop_label_offset_.x);
	render.mutable_stop_label_offset()->set_y(render_set.stop_label_offset_.y);

	std::stringstream ss;
	std::visit(svg::OutRgb{ ss }, render_set.underlayer_color_);

	render.set_underlayer_color_string(ss.str());
	render.set_underlayer_width(render_set.underlayer_width_);

	for (const auto& col : render_set.palette_) {
		std::stringstream ss2;
		std::visit(svg::OutRgb{ ss2 }, col);
	
		render.add_palette_string(ss2.str());
	}
}

void SomeGraph(graph_serialize::Graph& graph_ser,
	const graph::DirectedWeightedGraph<double>& graph) {

	for (const auto& edge : graph.GetVectorEdges()) {

		graph_serialize::Edge edge_ser;

		edge_ser.set_from(edge.from);
		edge_ser.set_to(edge.to);
		edge_ser.set_weight(edge.weight);

		*graph_ser.add_edges() = std::move(edge_ser);
	}

	for (const auto& edge_id : graph.GetList()) {

		graph_serialize::IncidenceList list;

		for (const auto id : edge_id) {
			list.add_edge_id(id);
		}	

		*graph_ser.add_incidience_lists() = std::move(list);
	}
}

void SerializationAllMapForGraph(graph_serialize::Graph& graph_ser,
	const transport_router::InitGraph& init) {

	graph_serialize::VertexToName vertex_to;

	for (const auto& [vertex, name] : init.GetVertexToName()) {

		vertex_to.add_id(vertex);
		vertex_to.add_name(name);
	}

	*graph_ser.mutable_vertex_to() = std::move(vertex_to);

	graph_serialize::NameToVertex name_to;

	for (const auto& [name, to_from] : init.GetNameToVertex()) {

		name_to.add_name(name);

		graph_serialize::VertexBeginAndEnd beg_end;
		beg_end.set_begin(to_from.begin);
		beg_end.set_end(to_from.end);

		*name_to.add_id() = std::move(beg_end);
	}

	*graph_ser.mutable_name_to() = std::move(name_to);

	graph_serialize::EdgeInfo info;

	for (const auto& [id, info_edge] : init.GetEdgeInfo()) {

		info.add_id(id);

		graph_serialize::EdgeForGraf for_graph;
		for_graph.set_bus_name(info_edge.bus_name);
		for_graph.set_span_count(info_edge.span_count);
		for_graph.set_time(info_edge.time);

		*info.add_edge() = std::move(for_graph);
	}

	*graph_ser.mutable_info() = std::move(info);
}

void SerializationGraph(const transport_router::InitGraph& init,
	graph_serialize::Graph& graph) {

	SomeGraph(graph, init.GetGraph());
	SerializationAllMapForGraph(graph, init);
}

void Serialization(std::ofstream& out,
	const InformationForCatalog& query,
	const renderer_for_set::RenderSettings& render_set,
	const transport_router::InitGraph& init
) {

	transport_set::Transport_set trans_list;
	graph_serialize::Graph graph;
	transport_set::Render_settings render;

	SerializationCatalog(query, trans_list);
	SerializationMap(render_set, render);
	SerializationGraph(init, graph);

	*trans_list.mutable_ren() = std::move(render);
	*trans_list.mutable_graph() = std::move(graph);

	trans_list.SerializeToOstream(&out);
}

InformationForCatalog DeserializeCatalog(transport_set::Transport_set& trans_list) {

	InformationForCatalog catalog;

	std::unordered_map<size_t, std::string> index_stops;

	for (int i = 0; i < trans_list.stops_size(); i++) {

		transport_catalogue::detail::BusStop new_stop;

		new_stop.name_bus_stop_ = trans_list.mutable_stops(i)->name_bus_stop();
		index_stops[i] = new_stop.name_bus_stop_;

		new_stop.cordinat_bus_stop_.lat = trans_list.mutable_stops(i)->cordinates().lat();
		new_stop.cordinat_bus_stop_.lng = trans_list.mutable_stops(i)->cordinates().lng();

		catalog.set_bus_stop.push_back(std::move(new_stop));
	}

	for (int i = 0; i < trans_list.buses_size(); i++) {

		transport_catalogue::detail::Bus new_bus;

		new_bus.number_bus_ = trans_list.mutable_buses(i)->number_bus();
		new_bus.circl = trans_list.mutable_buses(i)->circl();

		for (int j = 0; j < trans_list.buses(i).index_stops_size(); j++) {
			new_bus.bus_stop_.push_back(index_stops.at(trans_list.buses(i).index_stops(j)));
		}

		catalog.set_bus.push_back(std::move(new_bus));
	}

	for (int i = 0; i < trans_list.distance_size(); i++) {

		transport_catalogue::detail::DistanceToStop new_dist;

		new_dist.dist_in_meters_ = trans_list.mutable_distance(i)->distance();
		new_dist.name_stop_from_ = index_stops.at(trans_list.mutable_distance(i)->stop_from());
		new_dist.name_stop_to_ = index_stops.at(trans_list.mutable_distance(i)->stop_to());

		catalog.distance_.push_back(std::move(new_dist));
	}

	return catalog;
}

renderer_for_set::RenderSettings DeserializeMap(transport_set::Transport_set& render) {

	renderer_for_set::RenderSettings render_set;

	render_set.width_ = render.mutable_ren()->width();
	render_set.height_ = render.mutable_ren()->height();
	render_set.padding_ = render.mutable_ren()->padding();
	render_set.line_width_ = render.mutable_ren()->line_width();
	render_set.stop_radius_ = render.mutable_ren()->stop_radius();
	render_set.bus_label_font_size_ = render.mutable_ren()->bus_lable_font_size();
	render_set.bus_label_offset_.x = render.mutable_ren()->mutable_bus_lable_offset()->x();
	render_set.bus_label_offset_.y = render.mutable_ren()->mutable_bus_lable_offset()->y();
	render_set.stop_label_font_size_ = render.mutable_ren()->stop_lable_font_size();
	render_set.stop_label_offset_.x = render.mutable_ren()->mutable_stop_label_offset()->x();
	render_set.stop_label_offset_.y = render.mutable_ren()->mutable_stop_label_offset()->y();
	render_set.underlayer_color_ = render.mutable_ren()->underlayer_color_string();
	render_set.underlayer_width_ = render.mutable_ren()->underlayer_width();

	for (int i = 0; i < render.mutable_ren()->palette_string_size(); i++) {
		render_set.palette_.push_back(render.mutable_ren()->palette_string(i));
	}

	return render_set;
}

void DeserializeSomeGraph(transport_set::Transport_set& render, 
	transport_router::detail::InputSerialization& for_init) {

	for (int i = 0; i < render.graph().edges_size(); i++) {

		graph::Edge<double> new_edge;

		new_edge.from = render.mutable_graph()->mutable_edges(i)->from();
		new_edge.to = render.mutable_graph()->mutable_edges(i)->to();
		new_edge.weight = render.mutable_graph()->mutable_edges(i)->weight();

		for_init.edges.push_back(std::move(new_edge));
	}

	for (int i = 0; i < render.graph().incidience_lists_size(); i++) {

		std::vector<size_t> one;

		for (int j = 0; j < render.graph().incidience_lists(i).edge_id_size(); j++) {
			one.push_back(render.graph().incidience_lists(i).edge_id(j));
		}

		for_init.incidence_lists.push_back(std::move(one));
	}
}

void DeserializeMapForGraph(transport_set::Transport_set& render,
	transport_router::detail::InputSerialization& for_init) {

	for (int i = 0; i < render.graph().vertex_to().id_size(); i++) {
		for_init.vertex_to_name[render.graph().vertex_to().id(i)] = render.graph().vertex_to().name(i);
	}

	for (int i = 0; i < render.graph().name_to().name_size(); i++) {

		transport_router::detail::VertexBeginAndEnd name_to;
		name_to.begin = render.graph().name_to().id(i).begin();
		name_to.end = render.graph().name_to().id(i).end();
		for_init.name_to_vertex[render.graph().name_to().name(i)] = std::move(name_to);
	}

	for (int i = 0; i < render.graph().info().id_size(); i++) {

		transport_router::detail::EdgeForGraph new_info;

		new_info.bus_name = render.graph().info().edge(i).bus_name();
		new_info.span_count = render.graph().info().edge(i).span_count();
		new_info.time = render.graph().info().edge(i).time();

		for_init.edge_info[render.graph().info().id(i)] = std::move(new_info);
	}
}

transport_router::detail::InputSerialization DeserializeGraph(transport_set::Transport_set& render) {

	transport_router::detail::InputSerialization for_init;

	DeserializeSomeGraph(render, for_init);
	DeserializeMapForGraph(render, for_init);

	return for_init;
}