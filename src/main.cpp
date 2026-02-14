#include <iostream>
#include "init.h"

int main() {
    try {
        AppInit::start();
    } catch (const std::exception& e) {
        std::cerr << "程序异常: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}