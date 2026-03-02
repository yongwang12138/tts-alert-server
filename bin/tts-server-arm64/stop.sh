#!/bin/bash
# stop.sh - 一键停止并删除tts-alert-server.service自启服务

if [ "$(id -u)" -ne 0 ]; then
    echo "错误：请使用 sudo $0 运行本脚本"
    exit 1
fi

SERVICE_NAME="tts-alert-server.service"
SERVICE_PATH="/etc/systemd/system/$SERVICE_NAME"

echo -e "========== 开始清理 tts-alert-server 服务 ==========\n"

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

# 释放音频
echo -e "\n4. 释放音频设备..."
fuser -k /dev/snd/* 2>/dev/null

echo -e "\n========== 清理完成 =========="