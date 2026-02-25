#pragma once

#include <vector>
#include <atomic>
#include <functional>
#include <memory>
#include "miniaudio.h"

class AudioPlayer {
public:
    // 禁用拷贝构造和赋值
    AudioPlayer(const AudioPlayer&) = delete;
    AudioPlayer& operator=(const AudioPlayer&) = delete;

    // 获取单例实例
    static AudioPlayer& instance();

    // 初始化音频设备
    bool init(unsigned int sample_rate = 22050);

    // 播放 PCM 数据 (16-bit)
    bool playPCM16(const std::vector<int16_t>& pcm_data, unsigned int sample_rate);

    // 播放 PCM 数据 (float)
    bool playPCMFloat(const std::vector<float>& pcm_data, unsigned int sample_rate);

    // 停止播放
    void stop();

    // 检查是否正在播放
    bool isPlaying() const { return is_playing_; }

    // 设置播放完成回调
    void setPlaybackCompleteCallback(std::function<void()> callback) {
        complete_callback_ = std::move(callback);
    }

private:
    // 私有构造和析构
    AudioPlayer();
    ~AudioPlayer();

    // 设备回调
    static void dataCallback(ma_device* device, void* output, const void* input, ma_uint32 frame_count);

    // 内部播放函数
    bool playInternal(const std::vector<float>& pcm_data, unsigned int sample_rate);

    // 设备对象
    std::unique_ptr<ma_device> device_{nullptr};
    ma_device_config device_config_{};

    // 播放数据
    std::vector<float> playback_data_{};
    std::atomic<size_t> current_frame_{0};
    std::atomic<bool> is_playing_{false};

    // 回调
    std::function<void()> complete_callback_{nullptr};

    // 初始化标志
    std::atomic<bool> initialized_{false};
};