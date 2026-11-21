# TODO

本文件记录 `lingmo-builder` 的待办事项、配置缺陷与架构限制。条目按处理优先级排列，每条包含位置、现象、影响与处理方式。

## 待修复

### 1. `.getopt.sh` 的加载路径与文件位置不一致

- 位置：`build.sh` 加载 `.getopt.sh`；`.getopt.sh` 位于仓库根目录
- 现象：`build.sh` 通过 `$(dirname $0)/.getopt.sh` 加载选项表，`scripts/` 目录下不存在该文件
- 影响：在 `scripts/` 目录下直接执行 `build.sh` 时 `source` 失败，脚本终止
- 处理：将 `.getopt.sh` 移入 `scripts/`，或修改加载路径为 `$(dirname $0)/../.getopt.sh`。当前需在构建前手动复制

### 2. `variant-default` 的类型与目录检查不兼容

- 位置：`scripts/lingmo-config/variant-default`
- 现象：该路径为普通文件，内容为字符串 `variant-everything`；`build.sh` 通过 `[ -d ]` 判断变体是否存在，`auto/config` 通过 `cp -rTL` 复制变体目录
- 影响：目录检查失败时 `build.sh` 仅输出 `ERROR: Unknown variant of Lingmo live configuration`，不终止执行；`cp -rTL` 将该文件本身复制至 `config/`，最终 ISO 缺少 `variant-everything` 的软件包清单
- 处理：将 `variant-default` 转换为指向 `variant-everything` 的符号链接，或将 `build.sh` 的检查改为读取指针文件内容

### 3. `--debootstrap-options` 重复声明

- 位置：`scripts/auto/config` 的 `lb config` 调用
- 现象：`--debootstrap-options` 出现两次，第一次为 `--keyring=/usr/share/keyrings/debian-archive-keyring.gpg`，第二次为 `--include=apt-transport-https,ca-certificates,openssl`
- 影响：live-build 对同名选项采用后者覆盖前者的行为，第一次声明的 `--keyring` 参数不生效。当前构建依赖 `--keyring-packages debian-archive-keyring` 提供密钥，功能未受影响，但参数表与实际行为不一致
- 处理：合并为单次声明 `--debootstrap-options "--keyring=... --include=..."`

### 4. `purple` 变体缺少软件包清单

- 位置：`build.sh` 的 `installer` 分支，`elif [ "$LINGMO_VARIANT" = "purple" ]`
- 现象：分支引用 `lingmo-purple` profile 与 `installer-purple/packages`，仓库中不存在 `scripts/lingmo-config/installer-purple/` 目录
- 影响：执行 `--installer --variant purple` 时，生成 `simple-cdd/profiles/lingmo.downloads` 的步骤读取不存在的文件，构建终止
- 处理：补充 `installer-purple/packages`，或移除该分支

### 5. 注释与默认值不一致

- 位置：`scripts/build.sh`，`LINGMO_LAYERED_ROOTFS` 声明处
- 现象：上方注释说明分层功能未启用，代码默认值为 `yes`
- 影响：阅读注释可能得出与实现相反的结论
- 处理：更新注释，使其与默认值一致

### 6. 非 `default` 变体的镜像名缺少连字符

- 位置：`build.sh` 的 `target_image_name()`
- 现象：`LINGMO_VARIANT` 为 `default` 时输出 `canary-26a13`，其他取值时输出 `canary26a13`
- 影响：镜像名格式随变体变化，下游脚本按固定格式解析文件名时可能失败
- 处理：确认下游无格式依赖后统一插入连字符，或保持现状并在文档中说明

### 7. 构建号未接入计数器

- 位置：`scripts/build.sh` 第 23 行；`tools/build_version.sh`
- 现象：`LINGMO_BUILD_VER` 硬编码为 `26a13`，`LINGMO_BUILD_VER=$(generate_build_ver)` 已注释；`tools/build_version.sh` 的 `.version.build` 计数器仅由 `tools/test.sh` 使用
- 影响：同一版本号的重复构建产出相同文件名，无法通过文件名区分构建批次
- 处理：恢复调用 `generate_build_ver`，或明确将构建号维护在版本文件中

### 8. live 用户自启动项依赖外部软件包

- 位置：`includes.chroot_after_packages/usr/lib/live/config/1165-create-installer-shortcut`
- 现象：脚本执行 `cp /usr/share/applications/install-lingmo.desktop /home/lingmo/.config/autostart`，该文件由外部软件包提供，不在本仓库内
- 影响：提供该文件的软件包缺失时，`cp` 失败且不产生错误输出（脚本未启用 `set -e`），live 桌面不会自动启动安装程序
- 处理：为 `cp` 增加存在性检查与错误输出，或将 `.desktop` 文件纳入本仓库

### 9. `tools/` 目录包含未使用的脚本

- 位置：`tools/config.sh`、`tools/default.xml`、`tools/update_status.sh`
- 现象：三个文件由其他仓库引入，构建过程与 CI 均不调用
- 影响：增加仓库体积与维护面，易被误认为构建链路的一部分
- 处理：确认无外部引用后移除

## 架构限制

以下条目为设计或工具链的固有限制，非缺陷。

### 10. `live-build` 需使用 Lingmo 分支

- 原因：`LB_LIVE_DIR` 变量由 Lingmo 分支实现。上游 `live-build` 不支持该变量，介质目录固定为 `live/`
- 影响：使用上游版本时，介质目录与 `auto/config` 写入引导参数的 `live-media-path=core.lm` 不一致，镜像无法定位自身介质
- 校验：`build.sh` 检查最低版本 `1:20230502`；CI 断言版本字符串包含 `+lingmo`

### 11. `packages.chroot` 中的 polkit deb 仅支持 amd64

- 位置：`common/packages.chroot/`
- 现象：五个 deb 包的架构字段为 `amd64`
- 影响：`arm64`、`i386` 构建时这五个包安装失败
- 处理：为其他架构提供对应版本，或将 polkit 版本固定策略改为 APT pin

### 12. live 镜像不包含 XWayland

- 原因：`xwayland` 为 `cutefish-compositor` 的 `Recommends` 依赖，`auto/config` 使用 `--apt-recommends false`，该依赖不安装
- 影响：依赖 X11 显示的程序无法启动。Calamares 以 root 身份运行，无法访问用户的 Wayland socket，同样受影响
- 处理：在 `package-lists/` 中显式声明 `xwayland` 与 `x11-xserver-utils`（提供 `xhost`）

### 13. 分层硬链接受设备边界限制

- 位置：`20-make-layer-images.binary`
- 现象：chroot 中通过 bind mount 引入的少量文件位于其他设备，`cp -dl` 对整批操作失败
- 处理：钩子解析 `cp:` 错误行定位这些文件，仅对其回退为复制。不可改为整层复制，27 GB 量级的 rootfs 会产生大量 I/O 并耗尽磁盘空间

### 14. 目录创建不能使用 `cp --parents`

- 现象：`cp --parents` 处理目录时输出 `-r not specified` 并跳过
- 处理：钩子单独收集目录列表，通过 `mkdir -p` 创建

### 15. FHS 骨架目录需显式补充

- 现象：`/dev`、`/proc`、`/sys`、`/run`、`/mnt`、`/media` 被 `find -prune` 排除
- 影响：缺失这些目录时，initramfs 中 `mount --move /dev /root/dev` 失败，内核 panic
- 处理：钩子将上述目录显式加入 `system` 层

### 16. 清单比对需统一排序区域设置

- 现象：`allfiles` 与 dpkg 文件清单的排序顺序需一致，前导斜杠需一致
- 影响：排序区域设置不一致时 `comm` 输出 `file 1 is not in sorted order`，比对结果错误。历史版本曾将所有文件判定为非软件包所有
- 处理：两处均设置 `LC_ALL=C`

### 17. 钩子的两项中止条件

- 现象：某一层声明的路径数大于零但实际创建数为零，或某层文件总数低于 chroot 文件总数的一半时，构建中止
- 影响：`filesystem.squashfs` 保留，镜像不完整
- 处理：出现中止时检查 `layer-images.conf` 的规则与文件权限，不通过修改阈值绕过

### 18. `lb config` 调用的续行约束

- 现象：该调用为单条命令加续行符
- 影响：命令内部插入注释行或遗漏续行符时，后续参数被 shell 解析为新命令
- 处理：修改该调用时保持单命令结构

## 待办

- 恢复 `make-sums` 调用（`build.sh` 末尾已注释），为产物生成校验和文件
- 启用 `tools/build_version.sh` 的构建号计数器，替换 `LINGMO_BUILD_VER` 的硬编码值
- 评估恢复 OnlyOffice 安装（`lingmo-hacks.chroot` 中已注释，原因记录为体积增加 0.7–1 GB 与构建期网络依赖）
- 为 `arm64` 提供 `packages.chroot` 中 polkit 包的对应架构版本
- 补充 `installer-purple/packages`，或从 `build.sh` 移除该分支
