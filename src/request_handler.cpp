#include "request_handler.h"

namespace Handler {

	RequestHandler::RequestHandler(const catalogue::TransportCatalogue& db, 
		const renderer::MapRenderer& renderer,
		const transport_router::TransportRouter& router)
		: db_(db)
		, renderer_(renderer) 
		, router_(router) {
	}

	std::optional<catalogue::RouteInfo> RequestHandler::GetBusStat(const std::string_view& bus_name) const {
		return db_.GetRouteInfo(bus_name);
	}

	const std::unordered_set<domain::BusPtr>* RequestHandler::GetBusesByStop(const std::string_view& stop_name) const {
		return db_.GetStopInfo(stop_name);
	}

	svg::Document RequestHandler::RenderMap() const {
		return renderer_.GetSVG(db_.GetAllRoutes());
	}

	std::optional<std::vector<const transport_router::TravelProps*>> RequestHandler::GetOptimalRoute(std::string_view from, std::string_view to) const {
		return router_.FindRoute(from, to);
	}
    
} // namespace Handler