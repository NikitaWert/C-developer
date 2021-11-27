#include "transport_catalogue.h"

using namespace transport_catalogue;

detail::BusOnTheStop TransportCatalogue::FindBusStop(const std::string& query) const {

	if (!bus_stop_.count(query)) {
		return detail::BusOnTheStop{ "", {} };
	}
	else if (!all_buses_on_stop_.count(bus_stop_.at(query))) {
		return detail::BusOnTheStop{ query, {} };
	}

	return detail::BusOnTheStop{ query, all_buses_on_stop_.at(bus_stop_.at(query)) };
}

detail::BusRoute TransportCatalogue::FindBus(const std::string& query) const {

	if (bus_.count(query)) {

		std::set<std::string> name_unique_bus_stop(bus_.at(query)->bus_stop_.begin(), bus_.at(query)->bus_stop_.end());

		detail::BusRoute bus_query;

		bus_query.find_bus = true;
		bus_query.number_bus_ = bus_.at(query)->number_bus_;
		bus_query.unique_bus_stop = name_unique_bus_stop.size();
		bus_query.count_bus_stop = (bus_.at(query)->circl) ? bus_.at(query)->bus_stop_.size() :
			bus_.at(query)->bus_stop_.size() * 2 - 1;

		double dist = CountDistance(bus_.at(query)->bus_stop_.begin(), bus_.at(query)->bus_stop_.end());
		dist = (bus_.at(query)->circl) ? dist : dist * 2;

		bus_query.total_distance_in_meters = CountDistanceInMeters(bus_.at(query)->bus_stop_.begin(), bus_.at(query)->bus_stop_.end(), bus_.at(query)->circl);
		bus_query.curvature = (dist != 0) ? bus_query.total_distance_in_meters / dist : -1;

		return bus_query;
	}

	return detail::BusRoute(query);
}

const detail::BusStop* TransportCatalogue::GetInfoBusStop(const std::string& query) const {
	static const detail::BusStop empty_bus_stop{ "", {0,0} };
	return (bus_stop_.count(query)) ? bus_stop_.at(query) : &empty_bus_stop;
}

const detail::Bus* TransportCatalogue::GetInfoBus(const std::string& query) const {
	static const detail::Bus empty_bus{ "", false, {} };
	return (bus_.count(query)) ? bus_.at(query) : &empty_bus;
}

void TransportCatalogue::AddBus(const detail::Bus& new_bus) {
	based_bus_.push_back(new_bus);
	AddBusHelper();
}

void TransportCatalogue::AddBus(detail::Bus&& new_bus) {

	based_bus_.push_back(std::move(new_bus));
	AddBusHelper();
}

void TransportCatalogue::AddBusHelper() {
	bus_[based_bus_.back().number_bus_] = &based_bus_.back();

	for (const auto& stop : based_bus_.back().bus_stop_) {
		if (!bus_stop_.count(stop)) continue;
		all_buses_on_stop_[bus_stop_.at(stop)].insert(based_bus_.back().number_bus_);
	}
}

const std::deque<detail::BusStop>* TransportCatalogue::GetAllBusStop() const {
	return &based_bus_stop_;
}

void TransportCatalogue::AddBusStop(const detail::BusStop& new_bus_stop) {

	based_bus_stop_.push_back(new_bus_stop);
	bus_stop_[based_bus_stop_.back().name_bus_stop_] = &based_bus_stop_.back();
}

void TransportCatalogue::AddBusStop(detail::BusStop&& new_bus_stop) {

	based_bus_stop_.push_back(std::move(new_bus_stop));
	bus_stop_[based_bus_stop_.back().name_bus_stop_] = &based_bus_stop_.back();
}

void TransportCatalogue::AddBusStopHelper() {
	bus_stop_[based_bus_stop_.back().name_bus_stop_] = &based_bus_stop_.back();
}

void TransportCatalogue::AddDistanceBetweenStops(const detail::DistanceToStop& dist_to_stop) {

	const auto* from = bus_stop_.at(dist_to_stop.name_stop_from_);
	const auto* to = bus_stop_.at(dist_to_stop.name_stop_to_);

	distance_between_stops_[std::pair<const detail::BusStop*, const detail::BusStop*>{ from, to }] = dist_to_stop.dist_in_meters_;
}

void TransportCatalogue::AddDistanceBetweenStops(detail::DistanceToStop&& dist_to_stop) {

	const auto* from = bus_stop_.at(std::move(dist_to_stop.name_stop_from_));
	const auto* to = bus_stop_.at(std::move(dist_to_stop.name_stop_to_));

	distance_between_stops_[std::pair<const detail::BusStop*, const detail::BusStop*>{ from, to }] = dist_to_stop.dist_in_meters_;
}

bool TransportCatalogue::FindBusOnTheStop(const std::string& stop_name) const {

	return !all_buses_on_stop_.count(bus_stop_.at(stop_name));
}

double TransportCatalogue::FindDistanceBetweenStops(const std::string& from, const std::string& to) const {

	auto from_ = GetInfoBusStop(from);
	auto to_ = GetInfoBusStop(to);

	if (distance_between_stops_.count(std::make_pair(from_, to_))) {
		return distance_between_stops_.at(std::make_pair(GetInfoBusStop(from), GetInfoBusStop(to)));
	}
	return distance_between_stops_.at(std::make_pair(to_, from_));
}