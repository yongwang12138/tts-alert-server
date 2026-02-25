#define MINIAUDIO_IMPLEMENTATION

#include "audio_player.h"
#include <algorithm>
#include <cstring>

AudioPlayer::AudioPlayer() {
    // 初始化设备配置
    device_config_ = ma_device_config_init(ma_device_type_playback);
    device_config_.playback.format   = ma_format_f32;
    device_config_.playback.channels = 1;
    device_config_.sampleRate        = 22050;
    device_config_.dataCallback      = dataCallback;
    device_config_.pUserData         = this;
}

AudioPlayer::~AudioPlayer() {
    stop();
    if (device_) {
        ma_device_uninit(device_.get());
    }
}

AudioPlayer& AudioPlayer::instance() {
    static AudioPlayer instance;
    return instance;
}

bool AudioPlayer::init(unsigned int sample_rate) {
    if (initialized_) {
        return true;
    }

    // 更新采样率
    device_config_.sampleRate = sample_rate;

    // 创建设备
    device_ = std::make_unique<ma_device>();
    if (ma_device_init(nullptr, &device_config_, device_.get()) != MA_SUCCESS) {
        device_.reset();
        return false;
    }

    initialized_ = true;
    return true;
}

void AudioPlayer::dataCallback(ma_device* pDevice, void* pOutput, const void* /*pInput*/, ma_uint32 frameCount) {
    auto* player = static_cast<AudioPlayer*>(pDevice->pUserData);
    auto* output = static_cast<float*>(pOutput);

    if (!player->is_playing_) {
        // 没有播放时输出静音
        std::memset(output, 0, frameCount * sizeof(float));
        return;
    }

    const auto framesRemaining = player->playback_data_.size() - player->current_frame_;
    const auto framesToWrite = std::min(static_cast<size_t>(frameCount), framesRemaining);

    if (framesToWrite > 0) {
        // 复制数据
        std::memcpy(output,
                    player->playback_data_.data() + player->current_frame_,
                    framesToWrite * sizeof(float));
        player->current_frame_ += framesToWrite;
    }

    // 填充剩余静音
    if (framesToWrite < frameCount) {
        std::memset(output + framesToWrite, 0, (frameCount - framesToWrite) * sizeof(float));
    }

    // 播放完毕
    if (player->current_frame_  >= player->playback_data_.size()) {
        player->is_playing_  = false;

        if (player->complete_callback_) {
            player->complete_callback_();
        }
    }
}

bool AudioPlayer::playInternal(const std::vector<float>& pcm_data, unsigned int sample_rate) {
    if (pcm_data.empty()) {
        return false;
    }

    // 确保设备已初始化
    if (!initialized_ && !init(sample_rate)) {
        return false;
    }

    // 如果采样率不匹配，重新初始化
    if (device_config_.sampleRate != sample_rate) {
        if (device_) {
            ma_device_uninit(device_.get());
            device_.reset();
        }
        device_config_.sampleRate = sample_rate;
        device_ = std::make_unique<ma_device>();
        if (ma_device_init(nullptr, &device_config_, device_.get()) != MA_SUCCESS) {
            device_.reset();
            return false;
        }
    }

    // 停止当前播放（如果有）
    if (is_playing_.exchange(false)) {
        ma_device_stop(device_.get());
    }

    // 设置新数据
    playback_data_ = pcm_data;
    current_frame_ = 0;
    is_playing_ = true;

    // 开始播放
    if (ma_device_start(device_.get()) != MA_SUCCESS) {
        is_playing_ = false;
        return false;
    }

    return true;
}

bool AudioPlayer::playPCM16(const std::vector<int16_t>& pcmData, unsigned int sampleRate) {
    if (pcmData.empty()) {
        return false;
    }

    // 转换 int16_t 到 float (-32768..32767 -> -1.0..1.0)
    std::vector<float> floatData(pcmData.size());
    for (size_t i = 0; i < pcmData.size(); ++i) {
        floatData[i] = static_cast<float>(pcmData[i]) / 32768.0f;
    }

    return playInternal(floatData, sampleRate);
}

bool AudioPlayer::playPCMFloat(const std::vector<float>& pcmData, unsigned int sampleRate) {
    return playInternal(pcmData, sampleRate);
}

void AudioPlayer::stop() {
    if (is_playing_.exchange(false) && device_) {
        ma_device_stop(device_.get());
    }
}