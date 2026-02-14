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

        // 读取配置值
        if (auto server = tbl["server"].as_table()) {
            m_port = server->at_path("port").value_or(m_port);
        }

        if (auto logger = tbl["logger"].as_table()) {
            m_console_output = logger->at_path("console_output").value_or(m_console_output);
            m_file_output = logger->at_path("file_output").value_or(m_file_output);
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
        file << R"(# 服务器配置
[server]
# 监听端口
port = 8809

# 日志配置
[logger]
# 是否输出到控制台
console_output = true
# 是否输出到文件
file_output = true
)";
        file.close();
        std::cout << "Default config file created: " << path << std::endl;
    }
}