#pragma once

#include "geo.h"

#include <deque>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <string_view>
#include <tuple>
#include <utility>

namespace transport_catalogue {

	namespace detail {

		struct DistanceToStop {
			int dist_in_meters_;
			std::string name_stop_to_;
			std::string name_stop_from_;
		};

		struct BusStop {
			std::string name_bus_stop_;
			geo::Coordinates cordinat_bus_stop_;

			bool operator<(const BusStop& other) {
				return name_bus_stop_ < other.name_bus_stop_;
			}
		};

		struct BusForSer {
			std::string number_bus_;
			bool circl = false;
			std::vector<size_t> bus_stop_;
		};

		struct Bus {
			std::string number_bus_;
			bool circl = false;
			std::vector<std::string> bus_stop_;
		};

		struct BusRoute {
			std::string number_bus_;
			bool find_bus = false;
			double curvature = -1;
			size_t total_distance_in_meters = 0;
			size_t unique_bus_stop = 0;
			size_t count_bus_stop = 0;

			BusRoute() {}

			BusRoute(const std::string& number) : number_bus_(number) {}
		};

		struct BusOnTheStop {
			std::string name_stop;
			std::set <std::string> number_buses;
		};
	}

	class TransportCatalogue {
	public:

		TransportCatalogue() {}

		detail::BusOnTheStop FindBusStop(const std::string& query) const;

		detail::BusRoute FindBus(const std::string& query) const;

		const detail::BusStop* GetInfoBusStop(const std::string& query) const;

		const detail::Bus* GetInfoBus(const std::string& query) const;

		void AddBus(detail::Bus&& new_bus);

		void AddBus(const detail::Bus& new_bus);

		void AddBusStop(detail::BusStop&& new_bus_stop);

		void AddBusStop(const detail::BusStop& new_bus_stop);

		void AddDistanceBetweenStops(const detail::DistanceToStop& dust_to_stop);

		void AddDistanceBetweenStops(detail::DistanceToStop&& dist_to_stop);

		const std::deque<detail::BusStop>* GetAllBusStop() const;

		bool FindBusOnTheStop(const std::string& stop_name) const;

		double FindDistanceBetweenStops(const std::string& from, const std::string& to) const;

	private:

		struct HasherVoid {
			size_t operator()(const std::pair<const detail::BusStop*, const detail::BusStop*> from_to) const {

				size_t h1 = hasher(from_to.first);
				size_t h2 = hasher(from_to.second);

				return h1 + h2 * 37;
			}

			std::hash<const void*> hasher;
		};

		struct HasherString {
			size_t operator()(const std::string& name) const {
				return hasher(name);
			}

			std::hash<std::string> hasher;
		};

		struct HasherStop {
			size_t operator()(const detail::BusStop* name) const {

				size_t one_cord = hasher_doub(name->cordinat_bus_stop_.lat);
				size_t two_cord = hasher_doub(name->cordinat_bus_stop_.lng);
				size_t str = hasher_str(name->name_bus_stop_);

				return one_cord + two_cord * 13 + str * 13 * 13;
			}

			std::hash<std::string> hasher_str;
			std::hash<double> hasher_doub;
		};

		std::unordered_map<std::string, const detail::BusStop*, HasherString> bus_stop_;
		std::unordered_map<const detail::BusStop*, std::set<std::string>, HasherStop> all_buses_on_stop_;
		std::unordered_map<std::string, const detail::Bus*, HasherString> bus_;
		std::unordered_map <std::pair<const detail::BusStop*, const detail::BusStop*>, size_t, HasherVoid> distance_between_stops_;

		std::deque<detail::BusStop> based_bus_stop_;
		std::deque<detail::Bus> based_bus_;

		void AddBusHelper();

		void AddBusStopHelper();

		template<typename Iterator>
		double CountDistance(const Iterator begin, const Iterator end) const;

		template<typename Iterator>
		size_t CountDistanceInMeters(const Iterator begin, const Iterator end, const bool circl) const;

		bool FindDistanceInMeters(std::pair<const detail::BusStop*, const detail::BusStop*> dist) const {
			return distance_between_stops_.count(dist);
		}
	};

	template<typename Iterator>
	double TransportCatalogue::CountDistance(const Iterator begin, const Iterator end) const {

		if (std::distance(begin, end) < 2) return 0.0;
		if (begin == end) return 0;

		double dist = 0;
		auto back_iterator = begin;
		auto it = begin;
		it++;

		for (; it != end; it++) {
			if (bus_stop_.count(*back_iterator) && bus_stop_.count(*it)) {
				dist += ComputeDistance(bus_stop_.at(*back_iterator)->cordinat_bus_stop_,
					bus_stop_.at(*it)->cordinat_bus_stop_);
			}
			back_iterator = it;
		}

		return dist;
	}

	template<typename Iterator>
	size_t TransportCatalogue::CountDistanceInMeters(const Iterator begin, const Iterator end, const bool circl) const {

		if (std::distance(begin, end) < 2) return 0;

		size_t dist = 0;
		auto back_iterator = begin;
		auto it = begin;
		it++;

		for (; it != end; it++) {

			if (bus_stop_.count(*back_iterator) && bus_stop_.count(*it)) {

				const auto iter_from = bus_stop_.at(*back_iterator);
				const auto iter_to = bus_stop_.at(*it);

				if (FindDistanceInMeters(std::make_pair(iter_from, iter_to))) {
					dist += distance_between_stops_.at(std::make_pair(iter_from, iter_to));
				}
				else if (FindDistanceInMeters(std::make_pair(iter_to, iter_from))) {
					dist += distance_between_stops_.at(std::make_pair(iter_to, iter_from));
				}

				if (!circl) {
					if (FindDistanceInMeters(std::make_pair(iter_to, iter_from))) {
						dist += distance_between_stops_.at(std::make_pair(iter_to, iter_from));
					}
					else if (FindDistanceInMeters(std::make_pair(iter_from, iter_to))) {
						dist += distance_between_stops_.at(std::make_pair(iter_from, iter_to));
					}
				}
			}

			back_iterator = it;
		}

		return dist;
	}

}