#pragma once

#include <string>
#include <nlohmann/json.hpp>
#include "httplib.h"

using json = nlohmann::json;

// 业务状态码枚举
enum class StatusCode : int {
    SUCCESS = 0,    // 成功
    FAIL  = 7,      // 操作失败
    NOAUTH = 8      // 无权限
};

class BaseHandler {
public:
    static json successResponse(const json& data = json(), const std::string& message = "成功");
    static json errorResponse(const std::string& message, StatusCode code = StatusCode::FAIL, const json& data = json());
    
protected:
    static bool parseJsonBody(const httplib::Request& req, json& out_json, std::string& error_msg);
};