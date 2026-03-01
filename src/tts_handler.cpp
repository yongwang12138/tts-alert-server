#include "tts_handler.h"
#include "logger.h"
#include <ctime>

#include "audio_player.h"
#include "tts_engine.h"

void TTSHandler::handleTTS(const httplib::Request& req, httplib::Response& res) {
    logger::info("收到 /tts 请求");

    json json_data;
    std::string parse_error;

    // 解析JSON
    if (!BaseHandler::parseJsonBody(req, json_data, parse_error)) {
        auto response = errorResponse("JSON格式错误: " + parse_error, StatusCode::FAIL);
        res.set_content(response.dump(), "application/json");
        res.status = 200;
        return;
    }

    // 检查text字段
    if (!json_data.contains("text") || !json_data["text"].is_string()) {
        auto response = errorResponse("缺少或无效的 'text' 字段", StatusCode::FAIL);
        res.set_content(response.dump(), "application/json");
        res.status = 200;
        return;
    }

    std::string text = json_data["text"];

    logger::info("========== TTS 请求 ==========");
    logger::info("文本: " + text);
    logger::info("==============================");

    // 文本转语音
    auto& tts = TTSEngine::instance();
    auto pcm_data = tts.textToPcm(text);

    if (pcm_data.empty()) {
        logger::error("语音合成失败");
        auto response = errorResponse("语音合成失败", StatusCode::FAIL);
        res.set_content(response.dump(), "application/json");
        res.status = 200;
        return;
    }

    // 播放语音
    if (!AudioPlayer::instance().playPCM16(pcm_data, 16000)) {
        logger::error("播放失败");
        auto response = errorResponse("播放失败", StatusCode::FAIL);
        res.set_content(response.dump(), "application/json");
        res.status = 200;
        return;
    }

    logger::info("播放成功");
    
    // 成功返回
    auto response = successResponse(json(), "播放成功");
    res.set_content(response.dump(), "application/json");
    res.status = 200;
}