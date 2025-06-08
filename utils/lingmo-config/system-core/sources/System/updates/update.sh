#!/bin/bash

TAR_SCREEN_NAME="lingmo-plymouth.plymouth"
BACKUP_DIR="/usr/share/"

set_StatusForDaemon() {
    mv /usr/share/plymouth/themes/lingmo-plymouth/$TAR_SCREEN_NAME $BACKUP_DIR/$TAR_SCREEN_NAME
    cp /System/updates/update_screen.plymouth /usr/share/plymouth/themes/lingmo-plymouth/$TAR_SCREEN_NAME
    plymouthd
    systemctl stop sddm
}

set_Pkg_Install() {
    # 显示启动画面
    plymouth show-splash

    # 切换到系统升级模式
    plymouth change-mode --updates

    # 获取所有 .deb 文件
    DEB_FILES=(/var/cache/apt/archives/*.deb)
    # DEB_FILES=(/System/updates/pkgs/*.deb)
    TOTAL_DEBS=${#DEB_FILES[@]}
    PROGRESS_STEP=$(echo "(100 / $TOTAL_DEBS + 0.5) / 1" | bc)

    # 初始化进度
    CURRENT_PROGRESS=0

    # 安装每个 .deb 文件
    for DEB_FILE in "${DEB_FILES[@]}"; do
#        echo "Installing $DEB_FILE..."
        apt install "$DEB_FILE" -y
        # cat $DEB_FILE
        # sleep 2

        # 更新进度
        CURRENT_PROGRESS=$(echo "$CURRENT_PROGRESS + $PROGRESS_STEP" | bc)
        # echo $CURRENT_PROGRESS
        plymouth system-update --progress="$CURRENT_PROGRESS"
    done

    # 确保进度达到 100
    plymouth system-update --progress=100
    sleep 3
    plymouth change-mode --reboot
    sleep 3
    plymouth change-mode --shutdown
    plymouth quit
    mv $BACKUP_DIR/$TAR_SCREEN_NAME /usr/share/plymouth/themes/lingmo-plymouth/$TAR_SCREEN_NAME
    qdbus org.freedesktop.PowerManagement /org/freedesktop/PowerManagement org.freedesktop.PowerManagement Reboot
}

# 调用函数
set_StatusForDaemon
set_Pkg_Install
