#pragma once

#include "base_handler.h"

using json = nlohmann::json;

class TTSHandler : public BaseHandler {
public:
    static void handleTTS(const httplib::Request& req, httplib::Response& res);
};