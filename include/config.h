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
    int get_port() const { return m_port; }
    bool get_console_output() const { return m_console_output; }
    bool get_file_output() const { return m_file_output; }

private:
    Config() = default;
    ~Config() = default;

    void create_default_config(const std::string& path);

    // 配置值
    int m_port = 8809;
    bool m_console_output = true;
    bool m_file_output = true;
};

// 便捷宏
#define CONFIG Config::instance()