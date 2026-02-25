#pragma once
#include <string>
#include <toml++/toml.h>

class Config {
public:
    // 禁用拷贝构造和赋值
    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;

    // 获取单例实例
    static Config& instance();

    // 加载配置文件
    bool load(const std::string& config_path = "config.toml");

    // 获取配置值
    int get_port() const { return port_; }
    bool get_console_output() const { return console_output_; }
    bool get_file_output() const { return file_output_; }

    // TTS 配置
    int get_tts_speaker_id() const { return tts_speaker_id_; }
    float get_tts_speed() const { return tts_speed_; }

private:
    Config() = default;
    ~Config() = default;

    void create_default_config(const std::string& path);

    // 服务器配置
    int port_{8809};                    // 服务器端口

    // 日志配置
    bool console_output_{true};          // 控制台日志输出
    bool file_output_{true};              // 文件日志输出

    // TTS 配置
    int tts_speaker_id_{0};               // TTS 说话人ID
    float tts_speed_{1.0f};                // TTS 语速
};