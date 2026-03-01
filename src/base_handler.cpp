#include "base_handler.h"
#include "logger.h"

json BaseHandler::successResponse(const json& data, const std::string& message) {
    json response = {
        {"code", static_cast<int>(StatusCode::SUCCESS)},
        {"msg", message},
        {"data", data}
    };
    return response;
}

json BaseHandler::errorResponse(const std::string& message, StatusCode code, const json& data) {
    json response = {
        {"code", static_cast<int>(code)},
        {"msg", message},
        {"data", data}
    };
    return response;
}

bool BaseHandler::parseJsonBody(const httplib::Request& req, json& out_json, std::string& error_msg) {
    try {
        out_json = json::parse(req.body);
        return true;
    } catch (const std::exception& e) {
        error_msg = e.what();
        logger::error(std::string("JSON parse error: ") + e.what());
        return false;
    }
}