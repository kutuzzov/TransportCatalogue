#include "json_reader.h"
#include "json_builder.h" 

#include <vector>
#include <string>
#include <sstream>
/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */
namespace json_reader {
	using namespace std::literals;
	JsonReader::JsonReader(std::istream& input, DataBasePtr catalogue, MapRenderPtr map_renderer)
		: input_(json::Load(input))
		, catalogue_ptr_(catalogue)
		, map_renderer_ptr_(map_renderer) {
		ProcessInput_();
	}

	void JsonReader::PrintResult(std::ostream& output) {
		json::Print(GetOutput(), output);
	}

	json::Document JsonReader::GetOutput() {
		Handler::RequestHandler handler(*catalogue_ptr_, *map_renderer_ptr_);
		json::Array result;
		for (const json::Node& request : input_.GetRoot().AsDict().at("stat_requests"s).AsArray()) {
			std::string type = request.AsDict().at("type"s).AsString();
			if (type == "Bus"s) {
				result.push_back(std::move(GetRoutes_(handler, request)));
			}
			if (type == "Stop"s) {
				result.push_back(std::move(GetStops_(handler, request)));
			}
			if (type == "Map"s) {
				result.push_back(std::move(GetMap_(handler, request)));
			}
		}
		return json::Document(std::move(result));
	}

	json::Dict JsonReader::GetRoutes_(Handler::RequestHandler& handler, const json::Node& request) {
		auto route_info = handler.GetBusStat(request.AsDict().at("name"s).AsString());
		int id = request.AsDict().at("id"s).AsInt();
		if (route_info.has_value()) {
			return json::Builder{}
				.StartDict()
				.Key("request_id"s)
				.Value(id)
				.Key("curvature"s)
				.Value(route_info.value().curvature)
				.Key("route_length"s)
				.Value(route_info.value().route_length)
				.Key("stop_count"s)
				.Value(route_info.value().stops)
				.Key("unique_stop_count"s)
				.Value(route_info.value().unique_stops)
				.EndDict()
				.Build()
				.AsDict();
		}
		else {
			return json::Builder{}
				.StartDict()
				.Key("error_message"s)
				.Value("not found"s)
				.EndDict()
				.Build()
				.AsDict();
		}
	}

	json::Dict JsonReader::GetStops_(Handler::RequestHandler& handler, const json::Node& request) {
		auto stop_info = handler.GetBusesByStop(request.AsDict().at("name"s).AsString());
		int id = request.AsDict().at("id"s).AsInt();
		if (stop_info) {
			std::set<std::string> buses;
			for (auto* bus : *stop_info) {
				buses.insert(bus->name);
			}
			json::Array buses_arr;
			for (auto& bus : buses) {
				buses_arr.push_back(bus);
			}
			return json::Builder{}
				.StartDict()
				.Key("request_id"s)
				.Value(id)
				.Key("buses"s)
				.Value(buses_arr)
				.EndDict()
				.Build()
				.AsDict();
		}
		else {
			return json::Builder{}
				.StartDict()
				.Key("request_id"s)
				.Value(id)
				.Key("error_message"s)
				.Value("not found"s)
				.EndDict()
				.Build()
				.AsDict();
		}
	}

	json::Dict JsonReader::GetMap_(Handler::RequestHandler& handler, const json::Node& request) {
		renderer::RenderSettings render_settings = ProcessRenderSettings(input_.GetRoot().AsDict().at("render_settings"s).AsDict());
		int id = request.AsDict().at("id"s).AsInt();
		map_renderer_ptr_->SetRenderSettings(render_settings);
		std::stringstream stream;
		handler.RenderMap().Render(stream);
		json::Node result = json::Builder{}
			.StartDict()
			.Key("request_id"s)
			.Value(id)
			.Key("map"s)
			.Value(stream.str())
			.EndDict()
			.Build();
		return result.AsDict();
	}

	renderer::RenderSettings JsonReader::ProcessRenderSettings(const json::Dict& settings) {
		renderer::RenderSettings result;
		result.width = settings.at("width"s).AsDouble();
		result.height = settings.at("height"s).AsDouble();
		result.padding = settings.at("padding"s).AsDouble();
		result.line_width = settings.at("line_width"s).AsDouble();
		result.stop_radius = settings.at("stop_radius"s).AsDouble();
		result.bus_label_font_size = settings.at("bus_label_font_size"s).AsInt();
		result.bus_label_offset = svg::Point(settings.at("bus_label_offset"s).AsArray()[0].AsDouble(),
			settings.at("bus_label_offset"s).AsArray()[1].AsDouble());
		result.stop_label_font_size = settings.at("stop_label_font_size"s).AsInt();
		result.stop_label_offset = svg::Point(settings.at("stop_label_offset"s).AsArray()[0].AsDouble(),
			settings.at("stop_label_offset"s).AsArray()[1].AsDouble());
		result.underlayer_width = settings.at("underlayer_width"s).AsDouble();
		result.underlayer_color = ProcessColor(settings.at("underlayer_color"s));
		for (const auto& color : settings.at("color_palette"s).AsArray()) {
			result.color_palette.push_back(ProcessColor(color));
		}
		return result;
	}

	svg::Color JsonReader::ProcessColor(const json::Node& color) {
		if (color.IsString()) {
			return color.AsString();
		}
		if (color.AsArray().size() == 3) {
			return svg::Rgb(color.AsArray()[0].AsInt(), color.AsArray()[1].AsInt(), color.AsArray()[2].AsInt());
		}
		return svg::Rgba(color.AsArray()[0].AsInt(), color.AsArray()[1].AsInt(), color.AsArray()[2].AsInt(),
			color.AsArray()[3].AsDouble());
	}

	void JsonReader::ProcessInput_() {
		ProcessStops_();
		ProcessRoutes_();
	}

	void JsonReader::ProcessStops_() {
		std::vector<std::string> stop_names;
		for (const auto& request : input_.GetRoot().AsDict().at("base_requests"s).AsArray()) {
			if (request.AsDict().at("type"s) == "Stop"s) {
				auto request_map = request.AsDict();
				auto name = request_map.at("name"s).AsString();
				auto latitude = request_map.at("latitude"s).AsDouble();
				auto longitude = request_map.at("longitude"s).AsDouble();
				auto road_distance = request_map.at("road_distances"s).AsDict();
				stop_names.push_back(name);
				std::vector<std::pair<std::string, int>> near_stops;
				for (const auto& [stop_name, distance] : road_distance) {
					near_stops.push_back(std::make_pair(stop_name, distance.AsInt()));
				}
				catalogue_ptr_->AddStop(name, { latitude, longitude }, near_stops);
			}
		}

		for (auto stop : stop_names) {
			auto* from = catalogue_ptr_->FindStop(stop);
			for (auto& [to, distance] : from->stops_distance) {
				catalogue_ptr_->SetDistanceBetweenStops(from, catalogue_ptr_->FindStop(to), distance);
			}
		}
	}

	void JsonReader::ProcessRoutes_() {
		for (const auto& request : input_.GetRoot().AsDict().at("base_requests"s).AsArray()) {
			if (request.AsDict().at("type"s) == "Bus"s) {
				auto request_map = request.AsDict();
				const auto& name = request_map.at("name"s).AsString();
				std::vector<std::string_view> stops;
				for (const auto& stop : request_map.at("stops"s).AsArray()) {
					stops.push_back(stop.AsString());
				}
				catalogue_ptr_->AddRoute(name, stops, request_map.at("is_roundtrip"s).AsBool());
			}
		}
	}
}