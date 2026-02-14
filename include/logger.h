#pragma once

#include <string>

namespace logger {
    void init(bool to_console, bool to_file);

    // 添加日志级别函数
    void info(const std::string& msg);
    void debug(const std::string& msg);
    void warn(const std::string& msg);
    void error(const std::string& msg);
}