#include "request_handler.h"

/*
* Здесь можно было бы разместить код обработчика запросов к базе, содержащего логику, которую не
* хотелось бы помещать ни в transport_catalogue, ни в json reader.
*
* Если вы затрудняетесь выбрать, что можно было бы поместить в этот файл,
* можете оставить его пустым.
*/

namespace Handler {

	RequestHandler::RequestHandler(const catalogue::TransportCatalogue& db, const renderer::MapRenderer& renderer)
		: db_(db)
		, renderer_(renderer) {
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
}