#include "logger.h"
#include "plog/Log.h"
#include "plog/Appenders/ConsoleAppender.h"
#include "plog/Appenders/RollingFileAppender.h"
#include "plog/Formatters/TxtFormatter.h"
#include "plog/Initializers/ConsoleInitializer.h"
#include <filesystem>
#ifdef _WIN32
#include <windows.h>
#endif

namespace logger {
    void init(bool to_console, bool to_file) {
#ifdef _WIN32
        // 设置 Windows 控制台设置为 UTF-8
        SetConsoleOutputCP(CP_UTF8);
        SetConsoleCP(CP_UTF8);
#endif

        std::filesystem::create_directories("logs");

        // PLOG_CHAR_IS_UTF8 让 plog 正确处理 UTF-8 字符串
        static plog::RollingFileAppender<plog::TxtFormatter> fileAppender(
            "logs/server.log", 5 * 1024 * 1024, 3
        );
        static plog::ConsoleAppender<plog::TxtFormatter> consoleAppender;

        if (to_console && to_file) {
            plog::init(plog::debug, &consoleAppender).addAppender(&fileAppender);
        } else if (to_console) {
            plog::init(plog::debug, &consoleAppender);
        } else if (to_file) {
            plog::init(plog::debug, &fileAppender);
        }
    }

    void info(const std::string& msg) { LOG_INFO << msg; }
    void debug(const std::string& msg) { LOG_DEBUG << msg; }
    void warn(const std::string& msg) { LOG_WARNING << msg; }
    void error(const std::string& msg) { LOG_ERROR << msg; }
}