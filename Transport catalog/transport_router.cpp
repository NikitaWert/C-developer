#include "transport_router.h"

namespace transport_router {

    using namespace detail;

    InitGraph::InitGraph(transport_catalogue::TransportCatalogue& catalog, detail::InputSerialization&& ser) : catalog_(catalog),
        graph_(std::move(ser.edges), std::move(ser.incidence_lists)),
        vertex_to_name_(std::move(ser.vertex_to_name)),
        name_to_vertex_(std::move(ser.name_to_vertex)),
        edge_info_(std::move(ser.edge_info)) {}

    void InitGraph::InfoForGraph(const InformationForCatalog& queries) {
        using namespace std;

        CreatePointStops(queries.set_bus_stop, queries.routing_settings.bus_wait_time);

        for (auto& bus_route : queries.set_bus) {

            std::vector<std::string> stops(bus_route.bus_stop_);
          
            BuildLinkRoute(stops.begin(), stops.end()
                , bus_route.number_bus_, queries.routing_settings.bus_velocity);

            if (!bus_route.circl) { 
                BuildLinkRoute(stops.rbegin(), stops.rend()
                    , bus_route.number_bus_, queries.routing_settings.bus_velocity);
            }
        }

    }

    void InitGraph::CreatePointStops(const std::vector<transport_catalogue::detail::BusStop>& stops, double bus_wait_time) {

        vertex_to_name_.reserve(stops.size()*2);
        name_to_vertex_.reserve(stops.size());
        edge_info_.reserve(stops.size() * 2);

        for (const auto& stop : stops) {
            VertexBeginAndEnd new_stop = { number_vertex_++, number_vertex_++ };
            BuildLinkToStop(new_stop.begin, new_stop.end, bus_wait_time, 1, "Stop");
            vertex_to_name_[new_stop.begin] = stop.name_bus_stop_;
            vertex_to_name_[new_stop.end] = stop.name_bus_stop_;
            name_to_vertex_[stop.name_bus_stop_] = std::move(new_stop);
        }
    }

   void InitGraph::BuildLinkToStop(size_t vertex_f, size_t vertex_s, double mass, int span_count, const std::string& name_bus) {
        
        auto id = graph_.AddEdge(graph::Edge<double>{ vertex_f, vertex_s, mass});
        edge_info_[id] = detail::EdgeForGraph{ mass, span_count, name_bus };
    }

    const graph::DirectedWeightedGraph<double>& InitGraph::GetGraph() const {
        return graph_;
    }

    const transport_catalogue::TransportCatalogue& InitGraph::GetCatalog() const {
        return catalog_;
    }

    const std::string& InitGraph::GetNameStop(size_t vertex) const {
        return vertex_to_name_.at(vertex);
    }

    VertexBeginAndEnd InitGraph::GetVertex(const std::string& stop) const {
        return name_to_vertex_.at(stop);
    }

    detail::EdgeForGraph InitGraph::GetEdgeInfo(size_t id) const {
        return edge_info_.at(id);
    }

    const std::unordered_map<size_t, std::string>& InitGraph::GetVertexToName() const {
        return vertex_to_name_;
    }

    const std::unordered_map<std::string, detail::VertexBeginAndEnd, std::hash<std::string>>&
        InitGraph::GetNameToVertex() const {
        return name_to_vertex_;
    }

    const std::unordered_map<size_t, detail::EdgeForGraph>& InitGraph::GetEdgeInfo() const {
        return edge_info_;
    }

    void InitGraph::SetVertexToName(
        std::unordered_map<size_t, std::string>&& other) {
        vertex_to_name_ = std::move(other);
    }

    void InitGraph::SetNameToVertex(
        std::unordered_map<std::string, detail::VertexBeginAndEnd, std::hash<std::string>>&& other) {
        name_to_vertex_ = std::move(other);
    }

    void InitGraph::SetEdgeInfo(
        std::unordered_map<size_t, detail::EdgeForGraph>&& other) {
        edge_info_ = std::move(other);
    }

    std::optional<BuildRoute> TransportRouter::GetItemsRoute(const std::string& from_str, const std::string& to_str) const {

        const size_t from = catalog_.GetVertex(from_str).begin;
        const size_t to = catalog_.GetVertex(to_str).begin;

        auto way = std::move(router_.BuildRoute(from, to));

        if (!way.has_value()) return std::nullopt;

        BuildRoute out;
        out.time = (*way).weight;

        for (int i = 0; i < static_cast<int>((*way).edges.size()); i++) {

            const graph::Edge<double>& info = catalog_.GetGraph().GetEdge((*way).edges.at(i));

            if (catalog_.GetNameStop(info.from) == catalog_.GetNameStop(info.to)) {
                ItemRoute new_item_stop;
                new_item_stop.type = "Wait";
                new_item_stop.name = catalog_.GetNameStop(info.from);
                new_item_stop.time = info.weight;
                out.items.push_back(std::move(new_item_stop));
            }
            else {

                auto [mass, span_count, name] = catalog_.GetEdgeInfo((*way).edges.at(i));

                ItemRoute new_item_bus;
                new_item_bus.type = "Bus";
                new_item_bus.time = mass;
                new_item_bus.span_count = span_count;
                new_item_bus.name = name;
                out.items.push_back(std::move(new_item_bus));
            }
        }

        return std::make_optional(out);
    }



}