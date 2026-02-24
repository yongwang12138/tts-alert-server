#include "router.h"
#include "tts_handler.h"

void register_all_routes(httplib::Server& server)
{
    // 原有/hello路由：直接移到这里
    server.Get("/hello", [](const httplib::Request&, httplib::Response& res) {
        res.set_content("Hello World!", "text/plain");
    });

    // /tts 路由 - 直接调用 Handle 的方法
    server.Post("/tts", TTSHandler::handleTTS);
}
