cmake_minimum_required(VERSION 3.15)

project(tts_alert_server LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

# 启用 FetchContent
include(FetchContent)

# 设置第三方库下载目录
set(THIRD_PARTY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party)
# 创建第三方库目录
file(MAKE_DIRECTORY ${THIRD_PARTY_DIR})

# ============ tomlplusplus (header-only) ============
set(TOMLPLUSPLUS_VERSION 3.4.0)
FetchContent_Populate(
        tomlplusplus
        GIT_REPOSITORY https://github.com/marzer/tomlplusplus.git
        GIT_TAG v${TOMLPLUSPLUS_VERSION}
        SOURCE_DIR ${THIRD_PARTY_DIR}/tomlplusplus
        QUIET
)
include_directories(${THIRD_PARTY_DIR}/tomlplusplus/include)

# ============ plog (header-only) ============
FetchContent_Populate(
        plog
        GIT_REPOSITORY https://github.com/SergiusTheBest/plog.git
        GIT_TAG 1.1.11
        SOURCE_DIR ${THIRD_PARTY_DIR}/plog
        QUIET
)
include_directories(${THIRD_PARTY_DIR}/plog/include)

# ============ httplib (header-only) ============
set(HTTPLIB_VERSION 0.30.2)
FetchContent_Populate(
        httplib
        GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
        GIT_TAG v${HTTPLIB_VERSION}
        SOURCE_DIR ${THIRD_PARTY_DIR}/httplib
        QUIET
)
include_directories(${THIRD_PARTY_DIR}/httplib)

# 包含头文件目录
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# 收集源文件
file(GLOB SOURCES "${PROJECT_SOURCE_DIR}/src/*.cpp")

# 创建可执行文件
add_executable(${PROJECT_NAME} ${SOURCES})

if(WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE
            _WIN32_WINNT=0x0A00 # Windows 10 或更高版本
            PLOG_CHAR_IS_UTF8=1 # plog 使用 UTF-8 编码
    )

    # 添加必要的 Windows 库
    target_link_libraries(${PROJECT_NAME} PRIVATE
            Ws2_32      # Socket 支持
    )
endif()