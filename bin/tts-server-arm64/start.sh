#!/bin/bash
# start.sh - 一键配置tts_alert_server开机自启

# 检查是否以root运行
if [ "$(id -u)" -ne 0 ]; then
    echo "请使用 sudo $0 运行本脚本"
    exit 1
fi

# ==================== 自动识别程序路径 ====================
# 获取start.sh自身的绝对路径
START_SCRIPT_PATH=$(readlink -f "$0")
# 获取start.sh所在的目录
SCRIPT_DIR=$(dirname "$START_SCRIPT_PATH")
# 拼接tts_alert_server的绝对路径
TARGET_PROGRAM="$SCRIPT_DIR/tts_alert_server"

# 验证tts_alert_server是否存在
if [ ! -f "$TARGET_PROGRAM" ]; then
    echo "错误：在当前目录 [$SCRIPT_DIR] 下未找到 tts_alert_server！"
    echo "请确保 start.sh 和 tts_alert_server 放在同一目录下。"
    exit 1
fi

# 验证并添加执行权限
if [ ! -x "$TARGET_PROGRAM" ]; then
    echo "提示：tts_alert_server 缺少执行权限，正在添加..."
    chmod +x "$TARGET_PROGRAM"
    if [ $? -ne 0 ]; then
        echo "错误：无法为 $TARGET_PROGRAM 添加执行权限！"
        exit 1
    fi
fi
# ==========================================================

# 配置systemd服务参数
SERVICE_NAME="tts-alert-server.service"
SERVICE_PATH="/etc/systemd/system/$SERVICE_NAME"

# ========== 检查服务是否已存在/已自启 ==========
echo "检查服务状态..."
# 检查服务文件是否存在
if [ -f "$SERVICE_PATH" ]; then
    # 检查服务是否已启用（开机自启）
    if systemctl is-enabled --quiet "$SERVICE_NAME"; then
        echo "错误：$SERVICE_NAME 已配置开机自启！"
        echo "🔧 如需重新配置，请先执行：sudo ./stop.sh"
        exit 1
    fi
fi
# =====================================================

# 创建systemd服务文件
echo "正在创建systemd服务文件..."
cat > "$SERVICE_PATH" << EOF
[Unit]
Description=TTS Alert Server
After=multi-user.target sound.target
Requires=local-fs.target

[Service]
Type=simple
ExecStart=$TARGET_PROGRAM
Restart=always
RestartSec=3
User=root
Group=root

[Install]
WantedBy=multi-user.target
EOF

# 重新加载systemd配置并启用服务
echo "重新加载systemd配置..."
systemctl daemon-reload

echo "设置开机自启..."
systemctl enable "$SERVICE_NAME"

echo "启动服务..."
systemctl start "$SERVICE_NAME"

# 检查服务状态
echo -e "\n========== 服务状态 =========="
systemctl status "$SERVICE_NAME" --no-pager

echo -e "\n✅ 配置完成！"
echo "📌 程序路径：$TARGET_PROGRAM"
echo "🔧 常用命令："
echo "   启动服务：sudo systemctl start tts-alert-server"
echo "   停止服务：sudo systemctl stop tts-alert-server"
echo "   查看状态：sudo systemctl status tts-alert-server"
echo "   关闭自启：sudo systemctl disable tts-alert-server"
echo "   移除服务：sudo rm $SERVICE_PATH && sudo systemctl daemon-reload"