cmake_minimum_required(VERSION 3.15)

project(tts_alert_server LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 可执行文件输出目录
if(WIN32)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin_win64)
    message(STATUS "Platform: Windows 64")
else()
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|ARM64|arm64")
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin_arm64)
        message(STATUS "Platform: Linux ARM64")
    else()
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin_x64)
        message(STATUS "Platform: Linux x64")
    endif()
endif()



# ============ 第三方库管理 ============
# 启用 FetchContent
include(FetchContent)
# 设置第三方库下载目录
set(THIRD_PARTY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party)
# 创建第三方库目录
file(MAKE_DIRECTORY ${THIRD_PARTY_DIR})

# ---------- tomlplusplus(header-only)：TOML 配置文件解析库 ----------
set(TOMLPLUSPLUS_INCLUDE_DIR ${THIRD_PARTY_DIR}/tomlplusplus)
if(EXISTS ${TOMLPLUSPLUS_INCLUDE_DIR})
    message(STATUS "Found existing tomlplusplus at ${TOMLPLUSPLUS_INCLUDE_DIR}")
else ()
    message(STATUS "Downloading tomlplusplus to ${TOMLPLUSPLUS_INCLUDE_DIR} ...")
    set(TOMLPLUSPLUS_VERSION 3.4.0)
    FetchContent_Populate(
            tomlplusplus
            GIT_REPOSITORY https://github.com/marzer/tomlplusplus.git
            GIT_TAG v${TOMLPLUSPLUS_VERSION}
            SOURCE_DIR ${TOMLPLUSPLUS_INCLUDE_DIR}
            QUIET # 减少下载时的输出信息
    )
endif()
# ---------- nlohmann/json (header-only)：JSON 解析库 ----------
set(JSON_INCLUDE_DIR ${THIRD_PARTY_DIR}/nlohmann_json)
if(EXISTS ${JSON_INCLUDE_DIR})
    message(STATUS "Found existing nlohmann_json at ${JSON_INCLUDE_DIR}")
else ()
    message(STATUS "Downloading nlohmann_json to ${JSON_INCLUDE_DIR} ...")
    set(JSON_VERSION 3.11.3)
    FetchContent_Populate(
            nlohmann_json
            GIT_REPOSITORY https://github.com/nlohmann/json.git
            GIT_TAG v${JSON_VERSION}
            SOURCE_DIR ${JSON_INCLUDE_DIR}
            QUIET
)
endif()
# ---------- plog(header-only)：轻量级日志库 ----------
set(PLOG_INCLUDE_DIR ${THIRD_PARTY_DIR}/plog)
if(EXISTS ${PLOG_INCLUDE_DIR})
    message(STATUS "Found existing plog at ${PLOG_INCLUDE_DIR}")
else ()
    message(STATUS "Downloading plog to ${PLOG_INCLUDE_DIR} ...")
    FetchContent_Populate(
            plog
            GIT_REPOSITORY https://github.com/SergiusTheBest/plog.git
            GIT_TAG 1.1.11
            SOURCE_DIR ${PLOG_INCLUDE_DIR}
            QUIET
    )
endif()
# ---------- httplib(header-only)：C++ HTTP 服务器库 ----------
# 用于处理 HTTP 请求，支持 Server 和 Client
set(HTTPLIB_INCLUDE_DIR ${THIRD_PARTY_DIR}/httplib)
if(EXISTS ${HTTPLIB_INCLUDE_DIR})
    message(STATUS "Found existing httplib at ${HTTPLIB_INCLUDE_DIR}")
else ()
    message(STATUS "Downloading httplib to ${HTTPLIB_INCLUDE_DIR} ...")
    set(HTTPLIB_VERSION 0.30.2)
    FetchContent_Populate(
            httplib
            GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
            GIT_TAG v${HTTPLIB_VERSION}
            SOURCE_DIR ${HTTPLIB_INCLUDE_DIR}
            QUIET
)
endif()
# ---------- miniaudio(header-only)：音频播放库 ----------
set(MINIAUDIO_INCLUDE_DIR ${THIRD_PARTY_DIR}/miniaudio)
if(EXISTS ${MINIAUDIO_INCLUDE_DIR})
    message(STATUS "Found existing miniaudio at ${MINIAUDIO_INCLUDE_DIR}")
else ()
    message(STATUS "Downloading miniaudio to ${MINIAUDIO_INCLUDE_DIR} ...")
    FetchContent_Populate(
            miniaudio
            GIT_REPOSITORY https://github.com/mackron/miniaudio.git
            GIT_TAG 0.11.24
            SOURCE_DIR ${MINIAUDIO_INCLUDE_DIR}
            QUIET
    )
endif()
# ---------- summertts：文本转语音库 ----------
set(SUMMERTTS_INCLUDE_DIR ${THIRD_PARTY_DIR}/summertts)
if(EXISTS ${SUMMERTTS_INCLUDE_DIR})
    message(STATUS "Found existing summertts at ${SUMMERTTS_INCLUDE_DIR}")
else ()
    message(STATUS "Downloading summertts to ${SUMMERTTS_INCLUDE_DIR} ...")
    set(SUMMERTTS_VERSION 1.0.2)
    FetchContent_Populate(
            summertts
            GIT_REPOSITORY https://github.com/yongwang12138/summertts.git
            GIT_TAG v${SUMMERTTS_VERSION}
            SOURCE_DIR ${SUMMERTTS_INCLUDE_DIR}
            QUIET
    )
endif()



# ============ 项目构建 ============
# 收集源文件
file(GLOB SOURCES "${PROJECT_SOURCE_DIR}/src/*.cpp")

# 创建可执行文件
add_executable(${PROJECT_NAME} ${SOURCES})

# ---------- 头文件路径设置 ----------
target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${TOMLPLUSPLUS_INCLUDE_DIR}/include
        ${JSON_INCLUDE_DIR}/include
        ${PLOG_INCLUDE_DIR}/include
        ${HTTPLIB_INCLUDE_DIR}
        ${MINIAUDIO_INCLUDE_DIR}
        ${SUMMERTTS_INCLUDE_DIR}/lib/include
)

if(WIN32)
# ---------- 库文件路径设置 ----------
target_link_directories(${PROJECT_NAME} PRIVATE
        ${SUMMERTTS_INCLUDE_DIR}/bin_win64
)
endif()

# ---------- 链接库 ----------
target_link_libraries(${PROJECT_NAME} PRIVATE
        summertts
)

# ---------- Windows 平台特殊设置 ----------
if(WIN32)
    # 预处理器定义：相当于在代码中 #define 这些宏
    target_compile_definitions(${PROJECT_NAME} PRIVATE
            _WIN32_WINNT=0x0A00 # 指定目标 Windows 版本为 Windows 10 0x0A00 = Windows 10, 0x0600 = Vista, 0x0601 = Win7
            PLOG_CHAR_IS_UTF8=1 # plog 使用 UTF-8 编码
            CPPHTTPLIB_NO_GETADDRINFO_BY_NAME=1 # httplib 禁用异步DNS
            CPPHTTPLIB_USE_POLL=1 # httplib 使用 poll
    )

    # 添加必要的 Windows 库
    target_link_libraries(${PROJECT_NAME} PRIVATE
            Ws2_32 # Windows Socket API
    )
endif()