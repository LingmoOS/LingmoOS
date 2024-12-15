## Lingmo 桌面环境 {#mainpage}

Lingmo 桌面环境 (OCEAN) 依赖于 Lingmo 中提供的各种 Lingmo 桌面环境包，
如果你想要 Lingmo 桌面环境请使用这个

## 依赖

### 构建依赖

* lingmo-desktop-base | lingmo-desktop-server | lingmo-desktop-device
* lingmo-default-settings
* ocean-desktop
* ocean-dock
* ocean-launcher
* ocean-control-center(>> 2.90.5)
* startocean
* ocean-session-ui
* lingmo-artwork
* lingmo-manual
* ocean-file-manager
* ocean-qt5integration
* eject
* plymouth-theme-lingmo-logo
* lingmo-wallpapers
* fonts-noto
* ocean-kwin
* ocean-clipboard
* lingmo-screensaver
* lingmo-license-activator
* lingmo-app-store
* lingmo-terminal
* lingmo-defender
* lingmo-system-monitor
* lingmo-devicemanager
* ocean-printer
* ocean-calendar
* uos-service-support
* org.lingmo.chineseime [!sw_64]
* ocean-introduction
* lingmo-user-experience-daemon
* fcitx-config-gtk
* udcp-installer [amd64 arm64]
* grub-efi-arm64-signed [arm64]
* grub-efi-loongarch64-signed [loongarch64]
* uos-audit-dac
* permission-manager
* sec-cfg

### 推荐的依赖

* ocean-qt5wayland-plugin
* uos-remote-assistance
* org.lingmo.downloader
* lingmo-aiassistant
* lingmo-editor
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
* ttf-lingmo-opensymbol
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
* lingmo-shortcut-viewer
* crda
* network-manager-integration-plugins
* ocean-manual-content
* libtxc-dxtn-s2tc0 | libtxc-dxtn-s2tc
* libflashplugin-pepper
* x-display-manager
* lingmo-music
* lingmo-image-viewer
* lingmo-calculator
* lingmo-voice-note
* lingmo-screen-recorder
* lingmo-font-manager
* lingmo-draw
* lingmo-album
* lingmo-movie
* lingmo-deb-installer
* lingmo-compressor
* lingmo-mail
* lingmo-diskmanager
* lingmo-camera
* fcitx-sunpinyin
* org.lingmo.scanner
* fcitx-ui-classic
* ocean-dconfig-daemon

### 冲突
 ocean-control-center(<< 2.90.5),
 lingmo-desktop-environment-desktop,
 lingmo-desktop-environment-dock,
 lingmo-desktop-environment-launcher,
 dconfig-daemon

## 帮助

如果您遇到任何其他问题，您可能还会发现这些渠道很有用：

* [Lingmo 社区空间](https://matrix.to/#/#lingmo-space:matrix.org)
* [Forum](https://bbs.lingmo.org)
* [WiKi](https://wiki.lingmo.org/)

## 贡献指南

我们鼓励您报告问题并做出更改

* [Contribution guide for developers](https://github.com/lingmoos/developer-center/wiki/Contribution-Guidelines-for-Developers-en). (English)
* [开发者代码贡献指南](https://github.com/lingmoos/developer-center/wiki/Contribution-Guidelines-for-Developers) (中文)

## 开源许可证

lingmo-tool-kit 在 [GPL-2+](LICENSE) 下发布.
