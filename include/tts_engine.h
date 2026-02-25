#pragma once

#include <string>
#include <vector>
#include <cstdint>

class TTSEngine {
public:
    // 禁用拷贝构造和赋值
    TTSEngine(const TTSEngine&) = delete;
    TTSEngine& operator=(const TTSEngine&) = delete;

    // 获取单例实例
    static TTSEngine& instance();

    // 输入文本，输出PCM数据
    std::vector<int16_t> textToPcm(const std::string& text);

private:
    TTSEngine();
    ~TTSEngine();

    // SummerTTS 内部对象
    void* synthesizer_{nullptr};
    float* model_data_{nullptr};
    int32_t model_size_{0};
};