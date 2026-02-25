#include "config.h"
#include <fstream>
#include <iostream>
#include <filesystem>

Config& Config::instance() {
    static Config instance;
    return instance;
}

bool Config::load(const std::string& config_path) {
    try {
        if (!std::filesystem::exists(config_path)) {
            std::cerr << "Config file not found: " << config_path << std::endl;
            std::cerr << "Creating default config file..." << std::endl;
            create_default_config(config_path);
        }

        toml::table tbl = toml::parse_file(config_path);

        // 读取服务器配置
        if (auto server = tbl["server"].as_table()) {
            port_ = server->at_path("port").value_or(port_);
        }

        // 读取日志配置
        if (auto logger = tbl["logger"].as_table()) {
            console_output_ = logger->at_path("console_output").value_or(console_output_);
            file_output_ = logger->at_path("file_output").value_or(file_output_);
        }

        // 读取 TTS 配置
        if (auto tts = tbl["tts"].as_table()) {
            tts_speaker_id_ = tts->at_path("speaker_id").value_or(tts_speaker_id_);

            // 从 toml 读取 float 值需要特殊处理
            if (auto speed_val = tts->at_path("speed").as_floating_point()) {
                tts_speed_ = static_cast<float>(speed_val->get());
            }
        }

        return true;
    } catch (const toml::parse_error& err) {
        std::cerr << "Config parsing failed: " << err.description() << std::endl;
        return false;
    }
}

void Config::create_default_config(const std::string& path) {
    std::ofstream file(path);
    if (file.is_open()) {
        file << R"(
# 服务器配置
[server]
# 监听端口
port = 8809

# 日志配置
[logger]
# 是否输出到控制台
console_output = true
# 是否输出到文件
file_output = true

# TTS 语音合成配置
[tts]
# 说话人ID (0 = 默认女声, 1 = 男声, 2 = 童声等，具体取决于模型)
speaker_id = 0

# 语速 (数值越小越快)
# 0.5 = 快速
# 1.0 = 正常
# 1.5 = 慢速
speed = 0.8
)";
        file.close();
        std::cout << "Default config file created: " << path << std::endl;
    }
}