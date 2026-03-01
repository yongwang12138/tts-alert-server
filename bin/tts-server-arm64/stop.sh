#!/bin/bash
# stop.sh - 一键停止并删除tts-alert-server.service自启服务

# 检查是否以root运行
if [ "$(id -u)" -ne 0 ]; then
    echo "错误：请使用 sudo $0 运行本脚本"
    exit 1
fi

# 服务配置
SERVICE_NAME="tts-alert-server.service"
SERVICE_PATH="/etc/systemd/system/$SERVICE_NAME"

echo -e "========== 开始清理 tts-alert-server 服务 ==========\n"

# ========== 检查服务是否存在/是否自启 ==========
echo "检查服务状态..."
# 检查服务文件是否存在
if [ ! -f "$SERVICE_PATH" ]; then
    echo "错误：$SERVICE_NAME 服务文件不存在（未配置开机自启）！"
    exit 1
fi
# 检查服务是否已启用（开机自启）
if ! systemctl is-enabled --quiet "$SERVICE_NAME"; then
    echo "错误：$SERVICE_NAME 未配置开机自启，无需清理！"
    exit 1
fi
# =====================================================

# 1. 停止运行中的服务
echo "1. 停止 $SERVICE_NAME 服务..."
if systemctl is-active --quiet "$SERVICE_NAME"; then
    systemctl stop "$SERVICE_NAME"
    echo "   ✅ 服务已停止"
else
    echo "   ℹ️  服务未运行，无需停止"
fi

# 2. 取消开机自启
echo -e "\n2. 取消 $SERVICE_NAME 开机自启..."
if systemctl is-enabled --quiet "$SERVICE_NAME"; then
    systemctl disable "$SERVICE_NAME"
    echo "   ✅ 已取消开机自启"
else
    echo "   ℹ️  服务未设置自启，无需取消"
fi

# 3. 删除服务文件
echo -e "\n3. 删除服务配置文件 $SERVICE_PATH..."
if [ -f "$SERVICE_PATH" ]; then
    rm -f "$SERVICE_PATH"
    echo "   ✅ 服务文件已删除"
else
    echo "   ℹ️  服务文件不存在，无需删除"
fi

# 4. 重新加载systemd配置
echo -e "\n4. 重新加载systemd配置..."
systemctl daemon-reload
echo "   ✅ systemd配置已重新加载"

# 最终提示
echo -e "\n========== 清理完成 =========="
echo "📝 tts-alert-server 服务已彻底清理！"