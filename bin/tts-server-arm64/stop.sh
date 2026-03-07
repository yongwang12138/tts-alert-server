#!/bin/bash
# stop.sh - 一键停止并删除tts-server.service自启服务

if [ "$(id -u)" -ne 0 ]; then
    echo "错误：请使用 sudo $0 运行本脚本"
    exit 1
fi

SERVICE_NAME="tts-server.service" 
SERVICE_PATH="/etc/systemd/system/$SERVICE_NAME"

echo -e "========== 开始清理 tts-server 服务 ==========\n"

if [ ! -f "$SERVICE_PATH" ]; then
    echo "错误：$SERVICE_NAME 服务文件不存在！"
    exit 1
fi

# 停止服务
echo "1. 停止服务..."
systemctl stop "$SERVICE_NAME"
systemctl disable "$SERVICE_NAME"

# 删除服务文件
echo -e "\n2. 删除服务文件..."
rm -f "$SERVICE_PATH"

# 重新加载systemd
echo -e "\n3. 重新加载systemd配置..."
systemctl daemon-reload

# 释放音频设备
echo -e "\n4. 释放音频设备..."
fuser -k /dev/snd/* 2>/dev/null

# 可选：清理 PulseAudio 环境（如果服务异常退出导致 socket 残留）
echo -e "\n5. 检查并清理 PulseAudio 环境..."
KYLIN_UID=$(id -u kylin 2>/dev/null)
if [ -n "$KYLIN_UID" ] && [ -d "/run/user/$KYLIN_UID/pulse" ]; then
    echo "   kylin 用户的 PulseAudio socket 目录存在"
    # 不主动删除，只是检查
    ls -la "/run/user/$KYLIN_UID/pulse/" 2>/dev/null | grep -E "pid|native" || echo "   没有发现残留 socket 文件"
fi

echo -e "\n========== 清理完成 =========="
echo "💡 提示：如果之后需要重新配置，请运行 sudo ./start.sh"