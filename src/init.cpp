#include "init.h"
#include "router.h"
#include "logger.h"
#include "config.h"
#include <string>

void AppInit::start()
{
    // 加载配置
    if (!Config::instance().load()) {
        // 如果加载失败，使用默认值继续运行
        std::cerr << "Failed to load config, using default values" << std::endl;
    }

    // 从配置读取日志设置
    bool console_output = CONFIG.get_console_output();
    bool file_output = CONFIG.get_file_output();

    // 初始化日志
    logger::init(console_output, file_output);

    logger::info("服务器启动中...");

    // 获取端口配置
    int port = CONFIG.get_port();

    httplib::Server server;
    register_all_routes(server);

    std::string listen_msg = "服务器监听 0.0.0.0:" + std::to_string(port);
    logger::info(listen_msg);

    server.listen("0.0.0.0", port);
}