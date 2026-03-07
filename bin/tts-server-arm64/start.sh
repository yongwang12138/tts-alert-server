#!/bin/bash
# start.sh - 一键配置tts_server开机自启

# 检查是否以root运行
if [ "$(id -u)" -ne 0 ]; then
    echo "请使用 sudo $0 运行本脚本"
    exit 1
fi

# ==================== 自动识别程序路径 ====================
START_SCRIPT_PATH=$(readlink -f "$0")
SCRIPT_DIR=$(dirname "$START_SCRIPT_PATH")
TARGET_PROGRAM="$SCRIPT_DIR/tts_server"

# 验证tts_server是否存在
if [ ! -f "$TARGET_PROGRAM" ]; then
    echo "错误：在当前目录 [$SCRIPT_DIR] 下未找到 tts_server！"
    exit 1
fi

# 验证并添加执行权限
if [ ! -x "$TARGET_PROGRAM" ]; then
    chmod +x "$TARGET_PROGRAM"
fi
# ==========================================================

# 配置systemd服务参数
SERVICE_NAME="tts-server.service" 
SERVICE_PATH="/etc/systemd/system/$SERVICE_NAME"

# ========== 检查服务是否已存在/已自启 ==========
echo "检查服务状态..."
if [ -f "$SERVICE_PATH" ]; then
    if systemctl is-enabled --quiet "$SERVICE_NAME" 2>/dev/null; then
        echo "错误：$SERVICE_NAME 已配置开机自启！"
        echo "如需重新配置，请先执行：sudo ./stop.sh"
        exit 1
    fi
fi
# =====================================================

# 先停止可能正在运行的服务
if systemctl is-active --quiet "$SERVICE_NAME" 2>/dev/null; then
    echo "停止已运行的服务..."
    systemctl stop "$SERVICE_NAME"
fi

# 获取声卡信息
echo "检测音频设备..."
SOUND_CARD="1"  # 默认使用 card 1

# 确保 root 有音频权限
echo "确保 root 有音频权限..."
# 将 root 加入 audio 组（如果不在）
if ! groups root | grep -q audio; then
    usermod -a -G audio root
    echo "已将 root 加入 audio 组"
fi

# 释放可能被占用的音频设备
echo "释放音频设备..."
fuser -k /dev/snd/* 2>/dev/null
sleep 1

# 创建 ALSA 配置文件
echo "创建 ALSA 配置文件..."
cat > /etc/asound.conf << EOF
# 默认使用 card $SOUND_CARD
pcm.!default {
    type plug
    slave.pcm "hw:$SOUND_CARD,0"
}
ctl.!default {
    type hw
    card $SOUND_CARD
}
EOF

# 获取 kylin 用户的 UID
KYLIN_UID=$(id -u kylin)

# 创建systemd服务文件（使用kylin用户运行）
echo "正在创建systemd服务文件..."
cat > "$SERVICE_PATH" << EOF
[Unit]
Description=TTS Server
After=multi-user.target sound.target
Wants=sound.target

[Service]
Type=simple
User=kylin
Group=kylin

# 库路径 - 让程序能找到同目录的 libsummertts.so
Environment=LD_LIBRARY_PATH=$SCRIPT_DIR:\$LD_LIBRARY_PATH

# PulseAudio 环境变量 - 让程序能连接到 kylin 的音频服务
Environment=XDG_RUNTIME_DIR=/run/user/$KYLIN_UID
Environment=PULSE_RUNTIME_PATH=/run/user/$KYLIN_UID/pulse
Environment=PULSE_SERVER=unix:/run/user/$KYLIN_UID/pulse/native

# ALSA 环境变量（作为备选）
Environment=ALSA_CARD=$SOUND_CARD
Environment=ALSA_DEVICE=plughw:$SOUND_CARD,0
Environment=ALSA_PCM_CARD=$SOUND_CARD

ExecStart=$TARGET_PROGRAM
WorkingDirectory=$SCRIPT_DIR

# 确保音频权限
SupplementaryGroups=audio

# 重启策略
Restart=always
RestartSec=5

[Install]
WantedBy=multi-user.target
EOF

# 重新加载systemd配置
echo "重新加载systemd配置..."
systemctl daemon-reload

echo "设置开机自启..."
systemctl enable "$SERVICE_NAME"

echo "启动服务..."
systemctl start "$SERVICE_NAME"

# 等待程序启动
sleep 3

# 检查服务状态
echo -e "\n========== 服务状态 =========="
systemctl status "$SERVICE_NAME" --no-pager

# 检查音频设备
echo -e "\n========== 音频设备检查 =========="
if systemctl is-active --quiet "$SERVICE_NAME"; then
    echo "服务运行中，检查音频设备..."
    
    PID=$(systemctl show -p MainPID --value "$SERVICE_NAME")
    if [ "$PID" -gt 0 ] && [ "$PID" != "0" ]; then
        echo "进程 $PID 运行用户: kylin"
        
        # 检查音频设备访问
        if ls -la /proc/$PID/fd/ 2>/dev/null | grep -E "snd|dsp|audio" >/dev/null; then
            echo "✅ 进程 $PID 已打开音频设备"
        else
            echo "⚠️  进程 $PID 未检测到音频设备打开"
        fi
    fi
fi

echo -e "\n✅ 配置完成！"
echo "📌 程序路径：$TARGET_PROGRAM"
echo "📌 运行用户：kylin"
echo "📌 工作目录：$SCRIPT_DIR"
echo "📌 音频设备：card $SOUND_CARD (plughw:$SOUND_CARD,0)"
echo ""
echo "🔧 常用命令："
echo "   🚀 启动服务：sudo systemctl start tts-server"
echo "   🛑 停止服务：sudo systemctl stop tts-server"
echo "   📊 查看状态：sudo systemctl status tts-server"
echo "   🔄 重启服务：sudo systemctl restart tts-server"
echo "   📝 查看日志：sudo journalctl -u tts-server -f"
echo "   ⚙️  关闭自启：sudo systemctl disable tts-server"
echo "   🗑️  移除服务：sudo rm $SERVICE_PATH && sudo systemctl daemon-reload"
echo ""
echo "⚠️  注意：如果修改了服务文件，需要执行：sudo systemctl daemon-reload"