#include "router.h"

void register_all_routes(httplib::Server& server)
{
    // 原有/hello路由：直接移到这里
    server.Get("/hello", [](const httplib::Request&, httplib::Response& res) {
        res.set_content("Hello World!", "text/plain");
    });

    // 后续新增路由，直接在这个函数里加即可，无需动main.cpp
    // 示例：新增/info路由，返回服务信息
    server.Get("/info", [](const httplib::Request&, httplib::Response& res) {
        res.set_content("Screen Control Server v1.0\nC++17 + httplib", "text/plain");
    });

    // 示例：新增POST路由（如需）
    server.Post("/post_test", [](const httplib::Request& req, httplib::Response& res) {
        res.set_content("POST Request Received", "text/plain");
    });
}
