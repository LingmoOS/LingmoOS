## Deepin 桌面环境 {#mainpage}

Deepin 桌面环境 (DDE) 依赖于 Deepin 中提供的各种 Deepin 桌面环境包，
如果你想要 Deepin 桌面环境请使用这个

## 依赖

### 构建依赖

* deepin-desktop-base | deepin-desktop-server | deepin-desktop-device
* deepin-default-settings
* dde-desktop
* dde-dock
* dde-launcher
* dde-control-center(>> 2.90.5)
* startdde
* dde-session-ui
* deepin-artwork
* deepin-manual
* dde-file-manager
* dde-qt5integration
* eject
* plymouth-theme-deepin-logo
* deepin-wallpapers
* fonts-noto
* dde-kwin
* dde-clipboard
* deepin-screensaver
* deepin-license-activator
* deepin-app-store
* deepin-terminal
* deepin-defender
* deepin-system-monitor
* deepin-devicemanager
* dde-printer
* dde-calendar
* uos-service-support
* org.deepin.chineseime [!sw_64]
* dde-introduction
* deepin-user-experience-daemon
* fcitx-config-gtk
* udcp-installer [amd64 arm64]
* grub-efi-arm64-signed [arm64]
* grub-efi-loongarch64-signed [loongarch64]
* uos-audit-dac
* permission-manager
* sec-cfg

### 推荐的依赖

* dde-qt5wayland-plugin
* uos-remote-assistance
* org.deepin.downloader
* deepin-aiassistant
* deepin-editor
* pppoe
* gvfs-fuse
* smbclient
* python-smbc
* cups-filters
* printer-driver-all
* foomatic-db-compressed-ppds
* foomatic-db-engine
* openprinting-ppds
* libsane-hpaio
* linux-firmware
* wireless-tools
* ttf-deepin-opensymbol
* printer-driver-hpcups
* printer-driver-hpijs
* libmtp-runtime
* samsung-print
* open-vm-tools-desktop
* open-vm-tools
* usbmuxd
* ipheth-utils
* libimobiledevice-dev
* libimobiledevice-utils
* dnsutils
* libfile-mimeinfo-perl
* deepin-shortcut-viewer
* crda
* network-manager-integration-plugins
* dde-manual-content
* libtxc-dxtn-s2tc0 | libtxc-dxtn-s2tc
* libflashplugin-pepper
* x-display-manager
* deepin-music
* deepin-image-viewer
* deepin-calculator
* deepin-voice-note
* deepin-screen-recorder
* deepin-font-manager
* deepin-draw
* deepin-album
* deepin-movie
* deepin-deb-installer
* deepin-compressor
* deepin-mail
* deepin-diskmanager
* deepin-camera
* fcitx-sunpinyin
* org.deepin.scanner
* fcitx-ui-classic
* dde-dconfig-daemon

### 冲突
 dde-control-center(<< 2.90.5),
 deepin-desktop-environment-desktop,
 deepin-desktop-environment-dock,
 deepin-desktop-environment-launcher,
 dconfig-daemon

## 帮助

如果您遇到任何其他问题，您可能还会发现这些渠道很有用：

* [Deepin 社区空间](https://matrix.to/#/#deepin-space:matrix.org)
* [Forum](https://bbs.deepin.org)
* [WiKi](https://wiki.deepin.org/)

## 贡献指南

我们鼓励您报告问题并做出更改

* [Contribution guide for developers](https://github.com/linuxdeepin/developer-center/wiki/Contribution-Guidelines-for-Developers-en). (English)
* [开发者代码贡献指南](https://github.com/linuxdeepin/developer-center/wiki/Contribution-Guidelines-for-Developers) (中文)

## 开源许可证

deepin-tool-kit 在 [GPL-2+](LICENSE) 下发布.
