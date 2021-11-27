#pragma once

#include "transport_catalogue.h"
#include "svg.h"

#include <variant>
#include <vector>
#include <map>

namespace renderer_for_set {

	using Palette = std::vector<svg::Color>;
	using Point = svg::Point;
	using BusLabelOffset = svg::Point;
	using StopLabelOffset = svg::Point;

	struct RenderSettings {
		double width_;
		double height_;
		double padding_;
		double line_width_;
		double stop_radius_;
		int bus_label_font_size_;
		BusLabelOffset bus_label_offset_;
		int stop_label_font_size_;
		StopLabelOffset stop_label_offset_;
		svg::Color underlayer_color_;
		double underlayer_width_;
		Palette palette_;
	};

	class MapRenderer {

	public:

		MapRenderer() = default;

		MapRenderer(const MapRenderer& other) = delete;

		MapRenderer(RenderSettings&& render_set) : render_settings_(std::move(render_set)) {}

		void AddSettings(RenderSettings&& render_set);

		const svg::Document& GetSvgDocument();

		void MakePolyline(const std::vector<Point>& points, const std::string bus_name);

		void MakeCircle(const Point& centre);

		void MakeTextBus(const Point& centre, const std::string& data);

		void MakeTextBusStop(const Point& centre, const std::string& data);

		double GetWidth();

		double GetHeight();

		double GetPadding();

	private:

		size_t index_color_polyline = 0;
		RenderSettings render_settings_;
		std::vector<svg::Polyline> all_poly;
		std::vector<svg::Circle> all_c;
		std::vector<svg::Text> all_text;
		std::map<std::string, svg::Color> color_for_text;
		svg::Document doc_svg_;
	};

}
