#pragma once

#include <string>
#include <nlohmann/json.hpp>
#include "httplib.h"

using json = nlohmann::json;

class BaseHandler {
public:
    virtual ~BaseHandler() = default;

protected:
    // RESTful 标准响应格式
    static json successResponse(const json& data = nullptr, const std::string& message = "success");
    static json errorResponse(const std::string& message, int code = 400, const json& data = nullptr);

    // 工具方法：解析JSON并返回是否成功
    static bool parseJsonBody(const httplib::Request& req, json& out_json, std::string& error_msg);
};