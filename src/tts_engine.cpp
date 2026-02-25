#include "tts_engine.h"
#include <iostream>
#include "config.h"
#include "logger.h"

// SummerTTS 头文件
#include "SynthesizerTrn.h"
#include "utils.h"

// 模型路径
static char MODEL_PATH[] = "models/single_speaker_fast.bin";

TTSEngine::TTSEngine() {
    logger::info(std::string("Loading TTS model from: ") + MODEL_PATH);

    // 1. 加载模型
    model_size_ = ttsLoadModel(MODEL_PATH, &model_data_);
    if (model_size_ <= 0 || !model_data_) {
        logger::error("Failed to load model!");
        return;
    }

    // 2. 创建合成器
    try {
        synthesizer_ = new SynthesizerTrn(model_data_, model_size_);
        logger::info("TTS initialized successfully!");
    } catch (const std::exception& e) {
        logger::error(std::string("Failed to create synthesizer: ") + e.what());
        tts_free_data(model_data_);
        model_data_ = nullptr;
        model_size_ = 0;
    }
}

TTSEngine::~TTSEngine() {
    if (synthesizer_) {
        delete static_cast<SynthesizerTrn*>(synthesizer_);
        synthesizer_ = nullptr;
    }

    if (model_data_) {
        tts_free_data(model_data_);
        model_data_ = nullptr;
    }

    model_size_ = 0;
}

TTSEngine& TTSEngine::instance() {
    static TTSEngine instance;
    return instance;
}

std::vector<int16_t> TTSEngine::textToPcm(const std::string& text) {
    std::vector<int16_t> result;

    if (!synthesizer_) {
        logger::error("TTS not initialized!");
        return result;
    }

    // 直接从配置文件读取参数
    auto& config = Config::instance();
    int speaker_id = config.get_tts_speaker_id();
    float speed = config.get_tts_speed();

    // 调用合成
    int32_t ret_len = 0;
    auto* synthesizer = static_cast<SynthesizerTrn*>(synthesizer_);
    int16_t* wav_data = synthesizer->infer(text, speaker_id, speed, ret_len);

    if (wav_data && ret_len > 0) {
        result.assign(wav_data, wav_data + ret_len);
        tts_free_data(wav_data);
        logger::info(std::string("Synthesized ") + std::to_string(ret_len) + " samples");
    } else {
        logger::error("Synthesis failed!");
    }

    return result;
}