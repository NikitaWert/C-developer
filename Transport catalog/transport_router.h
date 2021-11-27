#pragma once

#include "graph.h"
#include "router.h"
#include "transport_catalogue.h"
#include "domain.h"

#include <map>
#include <vector>
#include <optional>

namespace transport_router {

	namespace detail {

		struct ItemRoute {
			std::string type;
			double time;
			std::string name;
			int span_count;
		};

		struct EdgeForGraph {
			double time;
			int span_count;
			std::string bus_name;
		};

		struct BuildRoute {
			double time;
			std::vector<ItemRoute> items;
		};

		struct VertexBeginAndEnd {
			size_t begin;
			size_t end;
		};

		struct InputSerialization {
			std::vector<graph::Edge<double>> edges;
			std::vector<std::vector<size_t>> incidence_lists;
			std::unordered_map<size_t, std::string> vertex_to_name;
			std::unordered_map<std::string, detail::VertexBeginAndEnd, std::hash<std::string>> name_to_vertex;
			std::unordered_map<size_t, detail::EdgeForGraph> edge_info;
		};
	}

	class InitGraph {

		using CollectionEdge = std::map<size_t, std::map<size_t, double>>;

	public:
		explicit InitGraph(transport_catalogue::TransportCatalogue& catalog, size_t size_graph) : catalog_(catalog), graph_(size_graph) {
		}

		explicit InitGraph(transport_catalogue::TransportCatalogue& catalog, detail::InputSerialization&& ser);

		void InfoForGraph(const InformationForCatalog& queries);

		const graph::DirectedWeightedGraph<double>& GetGraph() const;

		const transport_catalogue::TransportCatalogue& GetCatalog() const;

		const std::string& GetNameStop(size_t vertex) const;

		detail::VertexBeginAndEnd GetVertex(const std::string& stop) const;

		detail::EdgeForGraph GetEdgeInfo(size_t id) const;

		const std::unordered_map<size_t, std::string>& GetVertexToName() const;

		const std::unordered_map<std::string, detail::VertexBeginAndEnd, std::hash<std::string>>&
			GetNameToVertex() const;

		const std::unordered_map<size_t, detail::EdgeForGraph>& GetEdgeInfo() const;

		void SetVertexToName(std::unordered_map<size_t, std::string>&& other);

		void SetNameToVertex(
			std::unordered_map<std::string, detail::VertexBeginAndEnd, std::hash<std::string>>&& other);

		void SetEdgeInfo(std::unordered_map<size_t, detail::EdgeForGraph>&& other);

	private:
		transport_catalogue::TransportCatalogue& catalog_;
		graph::DirectedWeightedGraph<double> graph_;
		std::unordered_map<size_t, std::string> vertex_to_name_;
		std::unordered_map<std::string, detail::VertexBeginAndEnd, std::hash<std::string>> name_to_vertex_;
		std::unordered_map<size_t, detail::EdgeForGraph> edge_info_;
		size_t number_vertex_ = 0;

		void BuildLinkToStop(size_t vertex_f, size_t vertex_s, double mass, int span_count, const std::string& name_bus);

		void CreatePointStops(const std::vector<transport_catalogue::detail::BusStop>& stops, double bus_wait_time);

		template<typename It>
		void BuildLinkRoute(It begin, It end, const std::string& name_bus, double velocity) {

			static const double m_to_km = 1000.0;
			static const double hours_to_min = 60.0;

			for (It first = begin; std::next(first) != end; first++) {

				double sum_mass = 0;
				int span_count = 1;
				size_t from_vertex = name_to_vertex_.at(*first).end;

				for (It second = std::next(first), start = first; second != end; second++, start++) {

					size_t to_vertex = name_to_vertex_.at(*second).begin;

					sum_mass += ((catalog_.FindDistanceBetweenStops(*start, *second) / m_to_km)
						/ velocity) * hours_to_min;

					BuildLinkToStop(from_vertex, to_vertex, sum_mass, span_count++, name_bus);
				}
			}
		}

	};

	class TransportRouter {
	public:
		explicit TransportRouter(InitGraph& catalog) : catalog_(catalog), router_(catalog_.GetGraph()) {
		}

		std::optional<detail::BuildRoute> GetItemsRoute(const std::string& from_str, const std::string& to_str) const;

	private:
		InitGraph& catalog_;
		graph::Router<double> router_;
	};

}