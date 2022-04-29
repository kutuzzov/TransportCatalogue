#pragma once

#include "transport_catalogue.h"
#include "request_handler.h"
#include "json.h"
#include "domain.h"
#include "map_renderer.h"

namespace json_reader {

	using DataBasePtr = catalogue::TransportCatalogue*;
	using MapRenderPtr = renderer::MapRenderer*;
	class JsonReader {
	public:
		explicit JsonReader(std::istream& input, DataBasePtr catalogue, MapRenderPtr map_renderer);

		void PrintResult(std::ostream& output);
	private:
		json::Document input_;
		DataBasePtr catalogue_ptr_;
		MapRenderPtr map_renderer_ptr_;

		json::Document GetOutput();
		json::Dict GetRoutes_(Handler::RequestHandler& handler, const json::Node& request);
		json::Dict GetStops_(Handler::RequestHandler& handler, const json::Node& request);
		json::Dict GetMap_(Handler::RequestHandler& handler, const json::Node& request);
		renderer::RenderSettings ProcessRenderSettings(const json::Dict& settings);
		svg::Color ProcessColor(const json::Node& color);
		void ProcessInput_();
		void ProcessStops_();
		void ProcessRoutes_();
	};
}