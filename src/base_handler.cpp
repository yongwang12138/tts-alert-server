#include "base_handler.h"
#include "logger.h"

json BaseHandler::successResponse(const json& data, const std::string& message) {
    json response = {
        {"code", 200},
        {"message", message}
    };

    if (!data.is_null()) {
        response["data"] = data;
    }

    return response;
}

json BaseHandler::errorResponse(const std::string& message, int code, const json& data) {
    json response = {
        {"code", code},
        {"message", message}
    };

    if (!data.is_null()) {
        response["data"] = data;
    }

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