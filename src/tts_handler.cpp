#include "tts_handler.h"
#include "logger.h"
#include <ctime>

void TTSHandler::handleTTS(const httplib::Request& req, httplib::Response& res) {
    logger::info("Received /tts request");

    json json_data;
    std::string parse_error;

    // 使用基类的工具方法解析JSON
    if (!BaseHandler::parseJsonBody(req, json_data, parse_error)) {
        auto response = errorResponse("Invalid JSON format: " + parse_error, 400);
        res.set_content(response.dump(), "application/json");
        res.status = 400;
        return;
    }

    if (json_data.contains("text") && json_data["text"].is_string()) {
        std::string text = json_data["text"];

        // 使用 logger 打印
        logger::info("========== TTS Request ==========");
        logger::info("Text: " + text);
        logger::info("=================================");

        // TODO: 这里添加实际的 TTS 处理逻辑

        // 构造返回数据
        json data = {
            {"received_text", text},
            {"timestamp", std::time(nullptr)},
        };

        // 返回成功响应（RESTful 格式）- 直接使用基类的方法
        auto response = successResponse(data, "TTS request processed successfully");
        res.set_content(response.dump(), "application/json");
        res.status = 200;

    } else {
        // 返回参数错误 - 直接使用基类的方法
        auto response = errorResponse("Missing or invalid 'text' field", 400);
        res.set_content(response.dump(), "application/json");
        res.status = 400;
    }
}