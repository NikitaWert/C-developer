#include "request_handler.h"

namespace handler_for_set {

	const OptionalBus RequestHandler::GetBus(const std::string& bus_name) const {
		return *db_.GetInfoBus(bus_name);
	}

	const OptionalBusStop RequestHandler::GetBusStop(const std::string& stop_name) const {
		return *db_.GetInfoBusStop(stop_name);
	}

	std::vector<detail::Bus> RequestHandler::SortQueries(const std::vector<detail::Bus>& query) const {
		std::vector<detail::Bus> sort_queries;
		sort_queries.reserve(query.size());

		for (const auto& item : query) {
			if (item.bus_stop_.empty()) continue;
			sort_queries.push_back(item);
		}

		std::sort(sort_queries.begin(), sort_queries.end(),
			[](const detail::Bus& left, const detail::Bus& right) {
				return left.number_bus_ < right.number_bus_;
			});

		return sort_queries;
	}

	std::vector<geo::Coordinates> RequestHandler::AllBusStopInVector() const {

		std::vector<geo::Coordinates> all_stops_cordinats;
		all_stops_cordinats.reserve(db_.GetAllBusStop()->size());

		for (const auto& cordinat : *db_.GetAllBusStop()) {

			if (db_.FindBusOnTheStop(cordinat.name_bus_stop_)) continue;
			all_stops_cordinats.push_back(cordinat.cordinat_bus_stop_);
		}
		
		return all_stops_cordinats;
	}

	ProjectorAndSortQueries RequestHandler::GetSortQueries(const InformationForCatalog& query) const {
		
		std::vector<detail::Bus> sort_queries(std::move(SortQueries(query.set_bus)));		
		std::vector<geo::Coordinates> all_stops_cordinats(std::move(AllBusStopInVector()));

		sphere_projector::SphereProjector shphere_proj;
		shphere_proj.Calculation(all_stops_cordinats.begin()
			, all_stops_cordinats.end()
			, renderer_.GetWidth()
			, renderer_.GetHeight()
			, renderer_.GetPadding());

		return std::make_pair(std::move(sort_queries), std::move(shphere_proj));
	}

	void RequestHandler::AddPointAndPointsName(std::vector<geo::Coordinates>& polyline
		, std::vector<std::string>& stop_names
		, const detail::Bus& bus) const {

		const int max_count_stop = 100;
		polyline.reserve(max_count_stop);
		stop_names.reserve(max_count_stop);

		for (const auto& stop : bus.bus_stop_) {

			OptionalBusStop stop_to_poly = GetBusStop(stop);
			polyline.push_back(stop_to_poly.cordinat_bus_stop_);
			stop_names.push_back(stop_to_poly.name_bus_stop_);
		}
	}

	void RequestHandler::IfCircle(std::vector<geo::Coordinates>& polyline, bool circl) const {
		if (!circl) {
			for (int i = static_cast<int>(polyline.size()) - 2; i >= 0; i--) {
				polyline.push_back(polyline.at(i));
			}
		}
	}

	OnePolyline RequestHandler::GetPolylineWithPointsName(std::vector<svg::Point>& correct_polyline
		,std::vector<std::string>& stop_names) const {

		OnePolyline new_for_all_poly;
		new_for_all_poly.reserve(correct_polyline.size());
		for (size_t i = 0; i < correct_polyline.size(); i++) {
			new_for_all_poly.push_back(std::make_pair(std::move(stop_names[i]), std::move(correct_polyline[i])));
		}

		return new_for_all_poly;
	}

	AllPolylineWithPointsName RequestHandler::MakeAllBusRoute(const std::vector<detail::Bus>& sort_queries,
		const sphere_projector::SphereProjector& shphere_proj) const {

		AllPolylineWithPointsName all_poly;
		all_poly.reserve(sort_queries.size());

		for (const auto& bus : sort_queries) {

			std::vector<geo::Coordinates> polyline;
			std::vector<std::string> stop_names;

			AddPointAndPointsName(polyline, stop_names, bus);
			size_t pre_size = polyline.size();
			IfCircle(polyline, bus.circl);

			std::vector<svg::Point> correct_polyline;
			correct_polyline.reserve(polyline.size());

			for (const auto& p : polyline) {
				correct_polyline.push_back(shphere_proj(p));
			}

			renderer_.MakePolyline(correct_polyline, bus.number_bus_);

			correct_polyline.resize(pre_size);
			all_poly.push_back(std::move(GetPolylineWithPointsName(correct_polyline, stop_names)));
		}

		return all_poly;
	}

	void RequestHandler::MakeAllBusStopWithSignatures(AllPolylineWithPointsName& all_poly) const {

		const int max_point_reserve = 50;

		std::map<std::string, bool> find_stop;
		std::vector<std::pair<std::string, svg::Point>> all_bus_stop;
		all_bus_stop.reserve(max_point_reserve);

		for (size_t i = 0; i < all_poly.size(); i++) {
			for (auto& p : all_poly[i]) {

				if (find_stop.count(p.first)) continue;
				find_stop[p.first] = true;
				all_bus_stop.push_back(std::move(p));
			}
		}

		std::sort(all_bus_stop.begin(), all_bus_stop.end(),
			[](const std::pair<std::string, svg::Point>& left, const std::pair<std::string, svg::Point>& right) {
				return left.first < right.first;
			});

		for (const auto& item : all_bus_stop) {
			renderer_.MakeCircle(item.second);
		}

		for (const auto& bus_stop : all_bus_stop) {
			renderer_.MakeTextBusStop(bus_stop.second, bus_stop.first);
		}
	}

	void RequestHandler::MakeAllBusSignatures(const AllPolylineWithPointsName& all_poly,
		const std::vector<detail::Bus>& sort_queries) const {
		for (size_t i = 0; i < all_poly.size(); i++) {

			renderer_.MakeTextBus(all_poly[i][0].second, sort_queries[i].number_bus_);
			if (all_poly[i][0].first != all_poly[i].back().first) renderer_.MakeTextBus(all_poly[i].back().second, sort_queries[i].number_bus_);
		}
	}

	const svg::Document& RequestHandler::RenderMap(const InformationForCatalog& query) const {

		using namespace handler_for_set;

		auto [sort_queries, shphere_proj] = std::move(GetSortQueries(query));

		AllPolylineWithPointsName all_poly(std::move(MakeAllBusRoute(sort_queries, shphere_proj)));
		
		MakeAllBusSignatures(all_poly, sort_queries);

		MakeAllBusStopWithSignatures(all_poly);

		return renderer_.GetSvgDocument();
	}

}