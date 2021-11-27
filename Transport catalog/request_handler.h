#pragma once

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "svg.h"
#include "domain.h"
#include "log_duration.h"

#include <optional>
#include <vector>
#include <map>
#include <cmath>
#include <algorithm>

using namespace transport_catalogue;

namespace sphere_projector {

    inline const double EPSILON = 1e-6;

    class SphereProjector {
    public:

        SphereProjector(){}

        template <typename PointInputIt>
        void Calculation(PointInputIt points_begin, PointInputIt points_end, double max_width,
            double max_height, double padding) {

            padding_ = padding;

            if (points_begin == points_end) {
                return;
            }

            const auto [left_it, right_it]
                = std::minmax_element(points_begin, points_end, [](auto lhs, auto rhs) {
                return lhs.lng < rhs.lng;
                    });
            min_lon_ = left_it->lng;
            const double max_lon = right_it->lng;

            const auto [bottom_it, top_it]
                = std::minmax_element(points_begin, points_end, [](auto lhs, auto rhs) {
                return lhs.lat < rhs.lat;
                    });
            const double min_lat = bottom_it->lat;
            max_lat_ = top_it->lat;

            std::optional<double> width_zoom;
            if (std::abs(max_lon - min_lon_) > EPSILON) {
                width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
            }

            std::optional<double> height_zoom;
            if (std::abs(max_lat_ - min_lat) > EPSILON) {
                height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
            }

            if (width_zoom && height_zoom) {
                zoom_coeff_ = std::min(*width_zoom, *height_zoom);
            }
            else if (width_zoom) {
                zoom_coeff_ = *width_zoom;
            }
            else if (height_zoom) {
                zoom_coeff_ = *height_zoom;
            }
        }

        svg::Point operator()(geo::Coordinates coords) const {
            return { (coords.lng - min_lon_) * zoom_coeff_ + padding_,
                    (max_lat_ - coords.lat) * zoom_coeff_ + padding_ };
        }

    private:
        double padding_;
        double min_lon_ = 0;
        double max_lat_ = 0;
        double zoom_coeff_ = 0;
    };

}

namespace handler_for_set {

    using OptionalBus = const detail::Bus&;
    using OptionalBusStop = const detail::BusStop&;
    using OnePolyline = std::vector<std::pair<std::string, svg::Point>>;
    using ProjectorAndSortQueries = std::pair<std::vector<detail::Bus>, sphere_projector::SphereProjector>;
    using AllPolylineWithPointsName = std::vector<std::vector<std::pair<std::string, svg::Point>>>;

    class RequestHandler {
    public:

        RequestHandler(const TransportCatalogue& db, renderer_for_set::MapRenderer& ren) : db_(db),
            renderer_(ren) {
        }

        const OptionalBus GetBus(const std::string& bus_name) const;

        const OptionalBusStop GetBusStop(const std::string& stop_name) const;

        const svg::Document& RenderMap(const InformationForCatalog& query) const;

    private:

        const TransportCatalogue& db_;
        renderer_for_set::MapRenderer& renderer_;

        ProjectorAndSortQueries GetSortQueries(const InformationForCatalog& query) const;

        AllPolylineWithPointsName MakeAllBusRoute(const std::vector<detail::Bus>& sort_queries,
            const sphere_projector::SphereProjector& shphere_proj) const;

        void MakeAllBusStopWithSignatures(AllPolylineWithPointsName& all_poly) const;

        void MakeAllBusSignatures(const AllPolylineWithPointsName& all_poly,
            const std::vector<detail::Bus>& sort_queries) const;

        std::vector<detail::Bus> SortQueries(const std::vector<detail::Bus>& query) const;

        std::vector<geo::Coordinates> AllBusStopInVector() const;

        void AddPointAndPointsName(std::vector<geo::Coordinates>& polyline
            , std::vector<std::string>& stop_names
            , const detail::Bus& bus) const;

        OnePolyline GetPolylineWithPointsName(std::vector<svg::Point>& correct_polyline
            , std::vector<std::string>& stop_names) const;

        void IfCircle(std::vector<geo::Coordinates>& polyline, bool circl) const;
    };

}