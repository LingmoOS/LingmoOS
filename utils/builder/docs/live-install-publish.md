# Live 镜像、安装后清理和 SourceForge 同步（2026-11-09）

## Live 与安装后行为

- common 和 everything 镜像包含 `lingmo-initial-setup`，安装后的首次会话由 `lingmo-dm` 启动初始设置。
- 正常/安全启动参数与 Lingmo DM 均使用 lingmo 用户。
- 全局 XDG autostart 启动 /usr/bin/lingmo-live-installer。脚本仅在 /proc/cmdline 包含 boot=live 时执行，检查安装器可执行文件，使用 flock 防止并发启动。
- 废弃复制不存在的 install-lingmo.desktop 的旧逻辑，程序坞入口改为实际提供的 lingmo-installer-config.desktop。
- 安装器使用唯一密码规则 minLength: 1；密码确认仍必须一致。不使用 libpwquality 长度/复杂度/字典规则。
- 用户与时区在安装器设置。安装后的 DM 首次自动登录进入 `lingmo-initial-setup`，完成后写入用户标记；后续启动自动进入 `lingmo-wayland`。
- 安装结束清除 /etc/debian_chroot、Live 安装器自启动项、本地 file:/cdrom: 软件源；同时处理 .list 和 deb822 .sources，保留独立的远程仓库和签名设置。
- 清理模块从 Calamares globalstorage 获取 rootMountPoint，拒绝 / 和逃出目标系统的路径。清理本身不要求联网。

## SourceForge 发布配置

构建产物成功上传为 GitHub Actions artifact 后，main 的 push/手动构建触发独立 SourceForge 发布任务。PR 构建不访问发布密钥。GitHub Release 后续失败不会阻止已经构建成功的 ISO 同步。缺少 ISO 会直接使构建失败。

在 LingmoOS/lingmo-builder → Settings → Secrets and variables → Actions 设置：

| 类型 | 名称 | 内容 |
| --- | --- | --- |
| Variable | SF_USERNAME | 拥有项目上传权限的 SourceForge 用户名 |
| Variable | SF_PROJECT | SourceForge 项目短名称 |
| Variable | SF_REMOTE_DIR | 项目 Files 基础目录，本项目填写 release/iso/lithium；不要附带版本号 |
| Secret | SF_SSH_PRIVATE_KEY | 已在该 SourceForge 账户登记公钥的 SSH 私钥 |
| Secret | SF_KNOWN_HOSTS | 从可信渠道核实的 frs.sourceforge.net SSH 主机密钥记录 |

脚本将完整 .iso 上传至 /home/frs/project/<project>/<directory>/，成功后再上传对应 .iso.sha256。使用 rsync 普通临时文件及完成后重命名，不使用服务端不支持的 partial-dir/delay-updates；不删除远端其他文件。不要把私钥写入仓库。上传脚本通过 SFTP 逐级创建并进入目标版本目录，兼容 SourceForge 的旧版 rsync；上传错误会明确失败。

本轮只读检查发现仓库尚无上述 Actions 配置，因此尚未发生真实 SourceForge 上传。

## 验证

在包含 installer、installer-config、builder、installer-firstboot 的总工作区运行：

    bash lingmo-builder/tests/verify-workspace.sh

测试使用临时目标文件系统、模拟 rsync 和伪安装器，不修改宿主账户、软件源或发送网络上传。完整 ISO 生成、虚拟机安装和真实 SourceForge 下载校验仍需在配置齐全后执行。

最终验证：18 项测试通过，安装模块描述及 Python/Shell 语法检查通过；每轮完成后已关闭 WSL。尚未构建新 ISO、进行虚拟机安装或真实上传。

版本目录来自构建前已通过 lingmo-version check --json 校验的 source.build_ver，通过构建 job output 原样传递至发布任务。lingmo-version 的真实版本源是 base-files/debian/rules，不是时间戳标签。SF_REMOTE_DIR=release/iso/lithium，构建版本为 26b03 时上传至 release/iso/lithium/26b03；版本更新时自动变化，不需要再修改 Actions 变量。

## ISO 校验文件

构建后在 artifact 内生成 sha256sum.txt、sha512sum.txt、md5sum.txt，以及 sha256sum.README、sha512sum.README、md5sum.README。清单仅包含 ISO 文件名和校验值，生成后立即用对应工具 --check 自检。README 包含 Linux、Windows PowerShell 和 macOS 用法及中文说明。上传先完成 ISO，再上传六个校验/说明文件；Actions artifact 也包含这些文件。MD5 仅兼容用途，优先使用 SHA-256/SHA-512。校验值未签名，不能代替发布者签名。

SourceForge rsync 服务端不支持 --mkpath，因此先在 SFTP 批处理内逐级 -mkdir、cd；已存在目录可以复用，无法进入目录则停止上传。认证继续使用相同私钥及严格主机指纹校验；不关闭安全检查来规避 SSH 警告。

上传完成后通过 rsync --dry-run --checksum --itemize-changes 对远端同名文件进行内容一致性检查。发现差异或缺失则失败；即使上传返回码为零，只要日志报告丢弃文件，也判定失败。ISO 验证通过后才上传清单和 README，所有文件验证通过后才输出发布成功。目录已存在导致 SFTP mkdir 报 Failure 可忽略，但后续 cd 必须成功。
