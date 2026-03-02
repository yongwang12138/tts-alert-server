#!/bin/bash
# start.sh - 一键配置tts_alert_server开机自启

# 检查是否以root运行
if [ "$(id -u)" -ne 0 ]; then
    echo "请使用 sudo $0 运行本脚本"
    exit 1
fi

# ==================== 自动识别程序路径 ====================
START_SCRIPT_PATH=$(readlink -f "$0")
SCRIPT_DIR=$(dirname "$START_SCRIPT_PATH")
TARGET_PROGRAM="$SCRIPT_DIR/tts_alert_server"

# 验证tts_alert_server是否存在
if [ ! -f "$TARGET_PROGRAM" ]; then
    echo "错误：在当前目录 [$SCRIPT_DIR] 下未找到 tts_alert_server！"
    exit 1
fi

# 验证并添加执行权限
if [ ! -x "$TARGET_PROGRAM" ]; then
    chmod +x "$TARGET_PROGRAM"
fi
# ==========================================================

# 配置systemd服务参数
SERVICE_NAME="tts-alert-server.service"
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

# 创建systemd服务文件（使用root运行）
echo "正在创建systemd服务文件..."
cat > "$SERVICE_PATH" << EOF
[Unit]
Description=TTS Alert Server
After=multi-user.target sound.target
Wants=sound.target

[Service]
Type=simple
User=root
Group=root
ExecStart=$TARGET_PROGRAM
WorkingDirectory=$SCRIPT_DIR

# 音频环境变量
Environment=ALSA_CARD=$SOUND_CARD
Environment=ALSA_DEVICE=plughw:$SOUND_CARD,0
Environment=ALSA_PCM_CARD=$SOUND_CARD

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
        echo "进程 $PID 运行用户: root"
        
        # 检查音频设备访问
        if ls -la /proc/$PID/fd/ 2>/dev/null | grep -E "snd|dsp|audio" >/dev/null; then
            echo "✅ 进程 $PID 已打开音频设备"
        else
            echo "⚠️  进程 $PID 未检测到音频设备打开"
        fi
    fi
fi

# 测试音频输出
echo -e "\n========== 音频测试 =========="
echo "尝试播放测试音..."

# 先确保设备空闲
fuser -k /dev/snd/* 2>/dev/null
sleep 1

# 测试音频
speaker-test -D plughw:$SOUND_CARD,0 -t sine -f 1000 -l 1
if [ $? -eq 0 ]; then
    echo "✅ 音频测试成功（如果能听到声音）"
else
    echo "⚠️  音频测试失败，但服务可能仍可正常工作"
fi

echo -e "\n✅ 配置完成！"
echo "📌 程序路径：$TARGET_PROGRAM"
echo "📌 运行用户：root"
echo "📌 工作目录：$SCRIPT_DIR"
echo "📌 音频设备：card $SOUND_CARD (plughw:$SOUND_CARD,0)"