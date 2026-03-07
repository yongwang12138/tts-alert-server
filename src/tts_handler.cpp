#include "tts_handler.h"
#include "logger.h"

#include "audio_player.h"
#include "tts_engine.h"

#include <algorithm>
#include <chrono>
#include <cctype>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>
#include <utility>

namespace {
class TTSDispatchQueue {
public:
    static TTSDispatchQueue& instance() {
        static TTSDispatchQueue queue;
        return queue;
    }

    void enqueue(std::string text) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            queue_.push(std::move(text));
        }
        condition_.notify_one();
    }

private:
    TTSDispatchQueue() : worker_(&TTSDispatchQueue::run, this) {}

    ~TTSDispatchQueue() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            stopping_ = true;
        }
        condition_.notify_one();
        if (worker_.joinable()) {
            worker_.join();
        }
    }

    void run() {
        while (true) {
            std::string text;
            {
                std::unique_lock<std::mutex> lock(mutex_);
                condition_.wait(lock, [this]() { return stopping_ || !queue_.empty(); });

                if (stopping_ && queue_.empty()) {
                    return;
                }

                text = std::move(queue_.front());
                queue_.pop();
            }

            auto& tts = TTSEngine::instance();
            auto pcm_data = tts.textToPcm(text);

            if (pcm_data.empty()) {
                logger::error("异步语音合成失败");
                continue;
            }

            if (!AudioPlayer::instance().playPCM16(pcm_data, 16000)) {
                logger::error("异步播放失败");
                continue;
            }

            logger::info("异步播放已启动");

            // 串行播放：等待当前音频播放结束后再处理下一条。
            while (AudioPlayer::instance().isPlaying()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(20));
            }

            logger::info("异步播放已结束");
        }
    }

    std::mutex mutex_;
    std::condition_variable condition_;
    std::queue<std::string> queue_;
    bool stopping_{false};
    std::thread worker_;
};

bool isBlankText(const std::string& text) {
    return std::all_of(text.begin(), text.end(), [](const unsigned char ch) {
        return std::isspace(ch) != 0;
    });
}
} // namespace

void TTSHandler::handleTTS(const httplib::Request& req, httplib::Response& res) {
    logger::info("收到 /tts 请求");

    json json_data;
    std::string parse_error;

    if (!BaseHandler::parseJsonBody(req, json_data, parse_error)) {
        auto response = errorResponse("JSON格式错误: " + parse_error, StatusCode::FAIL);
        res.set_content(response.dump(), "application/json");
        res.status = 200;
        return;
    }

    if (!json_data.contains("text") || !json_data["text"].is_string()) {
        auto response = errorResponse("缺少或无效的 'text' 字段", StatusCode::FAIL);
        res.set_content(response.dump(), "application/json");
        res.status = 200;
        return;
    }

    std::string text = json_data["text"].get<std::string>();
    if (text.empty() || isBlankText(text)) {
        auto response = errorResponse("'text' 字段不能为空", StatusCode::FAIL);
        res.set_content(response.dump(), "application/json");
        res.status = 200;
        return;
    }

    logger::info("收到文本: " + text);

    TTSDispatchQueue::instance().enqueue(std::move(text));

    auto response = successResponse(json(), "请求已接收");
    res.set_content(response.dump(), "application/json");
    res.status = 200;
}
