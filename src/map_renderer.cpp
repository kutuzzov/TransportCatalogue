#include "map_renderer.h"

using namespace std::literals;

namespace renderer {

	bool IsZero(double value) {
		return std::abs(value) < EPSILON;
	}

	svg::Point SphereProjector::operator()(geo::Coordinates coords) const {
		return { (coords.lng - min_lon_) * zoom_coeff_ + padding_,
				(max_lat_ - coords.lat) * zoom_coeff_ + padding_ };
	}

	void MapRenderer::SetRenderSettings(RenderSettings& settings) {
		render_settings_ = std::move(settings);
	}

	svg::Document MapRenderer::GetSVG(const std::map<std::string_view, domain::BusPtr>& routes) const {
		svg::Document result;
		std::map<std::string_view, domain::StopPtr> unique_stops;
		for (const auto& [_, route] : routes) {
			for (const auto stop : route->stops) {
				unique_stops[stop->name] = stop;
			}
		}

		std::vector<geo::Coordinates> coordinates;
		for (const auto [_, stop_ptr] : unique_stops) {
			coordinates.push_back(stop_ptr->coordinates);
		}

		SphereProjector projector(coordinates.begin(), coordinates.end(), render_settings_.width, render_settings_.height, render_settings_.padding);

		size_t counter = 0;

		for (const auto& [_, route] : routes) {
			if (route->stops.empty()) {
				continue;
			}
			std::vector<domain::StopPtr> route_stops{ route->stops.begin(), route->stops.end() };
			if (!route->is_circular) {
				route_stops.insert(route_stops.end(), next(route->stops.rbegin()), route->stops.rend());
			}

			size_t color_index = counter % render_settings_.color_palette.size();

			svg::Polyline polyline;
			polyline.SetFillColor(svg::NoneColor)
				.SetStrokeColor(render_settings_.color_palette[color_index])
				.SetStrokeWidth(render_settings_.line_width)
				.SetStrokeLineCap(svg::StrokeLineCap::ROUND)
				.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

			for (const auto stop : route_stops) {
				polyline.AddPoint(projector(stop->coordinates));
			}
			result.Add(polyline);
			++counter;
		}

		counter = 0;
		for (const auto& [_, route] : routes) {
			if (route->stops.empty()) {
				continue;
			}
			size_t color_index = counter % render_settings_.color_palette.size();
			svg::Text underlayer;
			underlayer.SetData(route->name)
				.SetPosition(projector(route->stops.front()->coordinates))
				.SetOffset(render_settings_.bus_label_offset)
				.SetFillColor(render_settings_.underlayer_color)
				.SetStrokeColor(render_settings_.underlayer_color)
				.SetFontFamily("Verdana"s)
				.SetFontSize(render_settings_.bus_label_font_size)
				.SetFontWeight("bold"s)
				.SetStrokeWidth(render_settings_.underlayer_width)
				.SetStrokeLineCap(svg::StrokeLineCap::ROUND)
				.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

			svg::Text text;
			text.SetData(route->name)
				.SetPosition(projector(route->stops.front()->coordinates))
				.SetOffset(render_settings_.bus_label_offset)
				.SetFontSize(render_settings_.bus_label_font_size)
				.SetFontFamily("Verdana"s)
				.SetFontWeight("bold"s)
				.SetFillColor(render_settings_.color_palette[color_index]);

			result.Add(underlayer);
			result.Add(text);

			if (!route->is_circular && route->stops.front() != route->stops.back()) {
				result.Add(underlayer.SetPosition(projector(route->stops.back()->coordinates)));
				result.Add(text.SetPosition(projector(route->stops.back()->coordinates)));
			}
			++counter;
		}

		for (const auto& [_, stop] : unique_stops) {
			svg::Circle stop_symbol;
			stop_symbol.SetCenter(projector(stop->coordinates))
				.SetRadius(render_settings_.stop_radius)
				.SetFillColor("white"s);
			result.Add(stop_symbol);
		}

		for (const auto& [_, stop] : unique_stops) {
			svg::Text underlayer;
			underlayer.SetData(stop->name)
				.SetPosition(projector(stop->coordinates))
				.SetOffset(render_settings_.stop_label_offset)
				.SetFontSize(render_settings_.stop_label_font_size)
				.SetFontFamily("Verdana"s)
				.SetFillColor(render_settings_.underlayer_color)
				.SetStrokeColor(render_settings_.underlayer_color)
				.SetStrokeWidth(render_settings_.underlayer_width)
				.SetStrokeLineCap(svg::StrokeLineCap::ROUND)
				.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

			svg::Text text;
			text.SetData(stop->name)
				.SetPosition(projector(stop->coordinates))
				.SetOffset(render_settings_.stop_label_offset)
				.SetFontSize(render_settings_.stop_label_font_size)
				.SetFontFamily("Verdana"s)
				.SetFillColor("black"s);

			result.Add(underlayer);
			result.Add(text);
		}
		return result;
	}
}