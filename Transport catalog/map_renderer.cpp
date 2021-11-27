#include "map_renderer.h"

namespace renderer_for_set {

	const svg::Document& MapRenderer::GetSvgDocument() {
		return doc_svg_;
	}

	void MapRenderer::AddSettings(RenderSettings&& render_set) {
		render_settings_ = std::move(render_set);
	}

	void MapRenderer::MakePolyline(const std::vector<Point>& points, const std::string bus_name) {

		svg::Polyline new_poly;

		for (const auto& item : points) {
			new_poly.AddPoint(item);
		}

		color_for_text[bus_name] = render_settings_.palette_.at(index_color_polyline);

		new_poly.SetStrokeColor(render_settings_.palette_.at(index_color_polyline))
			.SetStrokeWidth(render_settings_.line_width_)
			.SetStrokeLineCap(svg::StrokeLineCap::ROUND)
			.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

		new_poly.SetFillColor("none");

		(index_color_polyline + 1 < render_settings_.palette_.size())
			? index_color_polyline++ : index_color_polyline = 0;

		all_poly.push_back(new_poly);

		doc_svg_.Add(all_poly.back());
	}

	void MapRenderer::MakeCircle(const Point& centre) {

		svg::Circle new_c;
		new_c.SetCenter(centre)
			.SetRadius(render_settings_.stop_radius_)
			.SetFillColor("white");

		all_c.push_back(new_c);

		doc_svg_.Add(all_c.back());
	}

	void MapRenderer::MakeTextBus(const Point& centre, const std::string& data) {

		svg::Text new_text;

		new_text.SetData(data)
			.SetStrokeColor(render_settings_.underlayer_color_)
			.SetStrokeWidth(render_settings_.underlayer_width_)
			.SetPosition(centre)
			.SetFontFamily("Verdana")
			.SetOffset(render_settings_.bus_label_offset_)
			.SetFontSize(render_settings_.bus_label_font_size_)
			.SetFontWeight("bold")
			.SetStrokeLineCap(svg::StrokeLineCap::ROUND)
			.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
			.SetFillColor(render_settings_.underlayer_color_);

		all_text.push_back(new_text);
		doc_svg_.Add(all_text.back());
		
		svg::Text new_text2;

		new_text2.SetPosition(centre)
			.SetOffset(render_settings_.bus_label_offset_)
			.SetFontFamily("Verdana")
			.SetFontSize(render_settings_.bus_label_font_size_)
			.SetFontWeight("bold")
			.SetData(data)
			.SetFillColor(color_for_text.at(data));

			all_text.push_back(new_text2);
		    doc_svg_.Add(all_text.back());
			
	}

	void MapRenderer::MakeTextBusStop(const Point& centre, const std::string& data) {

		svg::Text new_text;

		new_text.SetData(data)
			.SetFillColor(render_settings_.underlayer_color_)
			.SetStrokeColor(render_settings_.underlayer_color_)
			.SetStrokeWidth(render_settings_.underlayer_width_)
			.SetPosition(centre)
			.SetFontFamily("Verdana")
			.SetOffset(render_settings_.stop_label_offset_)
			.SetFontSize(render_settings_.stop_label_font_size_)
			.SetStrokeLineCap(svg::StrokeLineCap::ROUND)
			.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

		all_text.push_back(new_text);
		doc_svg_.Add(all_text.back());
		
		svg::Text new_text2;
		new_text2.SetFillColor("black")		
            .SetPosition(centre)
			.SetOffset(render_settings_.stop_label_offset_)
			.SetFontFamily("Verdana")
			.SetFontSize(render_settings_.stop_label_font_size_)
			.SetData(data);

		all_text.push_back(new_text2);
		doc_svg_.Add(all_text.back()); 
	}

	double MapRenderer::GetWidth() {
		return render_settings_.width_;
	}

	double MapRenderer::GetHeight() {
		return render_settings_.height_;
	}

	double MapRenderer::GetPadding() {
		return render_settings_.padding_;
	}

}