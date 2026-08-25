# lingmo-oobe

Lingmo OS 首次启动配置向导（OOBE）。

在系统安装后的首次启动时运行，用于创建用户账户、设置主机名、配置 SDDM 自动登录，
并在桌面环境启动之前清理 Live 会话用户。

## 架构

```
开机
  │
  ├── BIOS/UEFI
  ├── 引导器 (GRUB)
  ├── 内核
  └── systemd
        │
        ├── firstboot.service（如果 /etc/.firstboot-complete 不存在）
        │     └─ 写入 SDDM 自动登录配置（liveuser + OOBE 会话）
        │
        └── SDDM
              │
              ├── [首次启动] 自动登录 liveuser → OOBE 会话
              │     └─ lingmo-oobe（全屏卡片界面）
              │           ├─ 创建用户账户
              │           ├─ 设置主机名
              │           ├─ 写入 SDDM 自动登录（真实用户）
              │           ├─ 删除 liveuser
              │           ├─ 标记完成（/etc/.firstboot-complete）
              │           └─ 重启
              │
              └── [后续启动] 自动登录真实用户 → 桌面环境
```

## 页面说明

| 页面     | 说明                         |
|----------|------------------------------|
| 欢迎     | 品牌展示，Logo 和引导文字    |
| 创建用户 | 输入新用户名和密码           |
| 主机名   | 设置机器主机名               |
| 确认     | 应用前确认配置                |
| 安装     | 应用设置时的进度显示          |
| 完成     | 设置完成，重启按钮            |

## 启动流程

| 启动次数 | 操作                                           |
|----------|------------------------------------------------|
| 第 1 次  | `firstboot.service` 写入 SDDM 首次启动配置     |
|          | SDDM 自动登录 `liveuser` → 启动 `lingmo-oobe` |
|          | OOBE 创建真实用户，写入显示管理器配置          |
|          | 删除 `liveuser`，创建完成标记文件               |
|          | 系统重启                                       |
| 第 2 次+ | SDDM 自动登录真实用户 → 正常进入桌面           |

## 依赖

- Qt6（Widgets）
- CMake ≥ 3.16
- C++17 编译器
- SDDM 或 LightDM（运行时）
- polkit（首次启动免密 pkexec）

## 构建

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
sudo cmake --install .
```

构建 Debian 包：

```bash
dpkg-buildpackage -uc -us -b
```

## 安装路径

```
/usr/bin/lingmo-oobe                     – 主程序
/usr/lib/lingmo-oobe/oobe.sh             – 启动脚本
/usr/share/xsessions/lingmo-oobe.desktop – X11 会话入口
/usr/share/wayland-sessions/lingmo-oobe.desktop – Wayland 会话入口
/etc/xdg/autostart/oobe.desktop          – 备用自启动
/lib/systemd/system/firstboot.service    – 显示管理器前配置服务
/etc/polkit-1/rules.d/10-lingmo-oobe.rules – liveuser 免密 polkit 规则
```

## 文件说明

| 文件                     | 用途                           |
|--------------------------|--------------------------------|
| `CMakeLists.txt`         | 构建配置                       |
| `main.cpp`               | 入口点，翻译加载               |
| `OobeWindow.h/.cpp`      | 主窗口和所有页面逻辑           |
| `oobe.sh`                | 会话启动器 / 首次启动配置      |
| `oobe.desktop`           | XDG 自启动备用                 |
| `firstboot.service`      | systemd 预显示管理器配置单元   |
| `lingmo-oobe.rules`      | polkit 免密规则                |
| `xsessions/*.desktop`    | 显示管理器会话入口             |
| `translations/*.ts`      | Qt 翻译源文件                  |
| `debian/`                | Debian 打包                    |

## 翻译

翻译文件位于 `translations/` 目录下，使用 Qt `.ts` 格式。添加新语言：

```bash
# 将 .ts 文件添加到 CMakeLists.txt 的 TS_FILES 中
# 然后构建（需要 qt6-linguist-tools）
lupdate lingmo-oobe_<lang>.ts
# 编辑 .ts 文件填写翻译
lrelease lingmo-oobe_<lang>.ts
```

## 开源协议

GNU General Public License v3.0 或更高版本（GPL-3.0-or-later）。

详见 `debian/copyright`。
