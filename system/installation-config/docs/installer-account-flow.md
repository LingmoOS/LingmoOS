# 安装与首次启动（2026-11-09）

流程：欢迎 → 地区与时区 → 软件选择 → 分区 → 创建用户 → 摘要 → 安装 → 完成。

密码仅要求非空（minLength: 1），两次输入必须匹配；不再使用 libpwquality。自动登录由用户选择。

builder 不再附带 lingmo-oobe。displaymanager 写入安装账户并直接使用 lingmo-wayland，会保留其他 DM 配置，不再记录或进入 OOBE 会话。services-systemd 不启用 firstboot.service。

sources-media、sources-media-unmount 和 sources-final 使用 Calamares rootMountPoint，不再根据宿主 mount 文本猜测目标目录。临时源卸载不执行 apt update；最终清除所有启用的 file:/cdrom: 源（包含 .list、.sources 和混合 URI），保留远程 Lingmo 源。移除 /etc/debian_chroot 和 Live 专用安装器自启动文件，避免安装后仍显示 (live) 或启动安装器。

需同步交付 lingmo-installer、lingmo-installer-config 和 lingmo-builder。旧 OOBE 包仅保留旧版本兼容用途，不属于新镜像启动链路。
