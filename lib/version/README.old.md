# lingmo-version

Lingmo OS 系统版本号统一工具：**只改一处，处处一致**。

## 为什么要它

系统版本信息原本散落在多个仓库的多个文件里，改一次要手工改十几处，而且很容易漏：

| 位置 | 说明 |
|---|---|
| `base-files/debian/rules` | **唯一源**：`DISTRO_NAME / VERSION / CODENAME / codename / DEBIAN_VER / BUILD_VER / BUILD_TIME / VARIANT / variant_id` |
| `base-files/etc/os-release` 等模板 | 由 rules 用 `@占位符@` 生成（`/etc/issue`、`/etc/issue.net`、`/etc/debian_version`、`motd`、`/system/*`、`/vendor/*`） |
| `lingmo-builder/.../hooks/live/lingmo-hacks.chroot` | 自带一份变量块，历史上还写死过 `VERSION_TYPE="development"` |
| `lingmo-builder/scripts/build.sh` | ISO 文件名用的 `LINGMO_VERTYPE / LINGMO_SYS_VERSION / LINGMO_BUILD_VER / RELEASE` |
| `lingmo-builder/.../includes.chroot_after_packages/etc/issue(.net)` | chroot 覆盖层，会盖过 base-files 的版本 |
| `lingmo-desktop-base/files/os-version.in` | **只校验**：`@@DISTRO_NAME@@` / `@@VERSION@@` / `@@BUILD_VER@@` 占位符必须保留（由打包与 live hook 替换成 `/etc/os-version`；被写死就会与系统版本脱节）|
| `lingmo-live/calamares/branding/lingmo/branding.desc` | **安装器界面显示的版本**（`version` / `shortVersion` / `versionedName` / `shortVersionedName` / `productName`） |

实际情况就是：同一台机器上同时存在 `3.0.6 / 25R07 / Helium`、`5.0.0`、`5.0` 三种说法。
本工具把 `base-files/debian/rules` 当**唯一源**，其余全部由它派生。

## 依赖

Python 3.8+，**只用标准库**，无需安装任何第三方包。

## 用法

在**同时含有 `base-files/` 与 `lingmo-builder/` 的目录**下执行：

```bash
lingmo-version check                       # 只检查，列出差异；有漂移退出码 1
lingmo-version sync                        # 按源同步（默认 dry-run，只打印要改什么）
lingmo-version sync --write                # 真正写入
lingmo-version sync --write --stamp        # 顺便刷新 BUILD_TIME

# 改版本：改源 + 同步 +（可选）bump base-files changelog
lingmo-version set 5.1 Lithium 26R01 --debian-ver 13.7 --write --bump-changelog

lingmo-version check --json                # 给 CI 用
lingmo-version check --root /path/to/parent   # 指定根目录（默认当前目录）
lingmo-version check --live /path/to/lingmo-live        # 指定 lingmo-live 位置
```
三个仓库的位置都可以用 `--base-files` / `--builder` / `--live` 覆盖（支持绝对路径，
CI 里很常用）；`lingmo-live` 不存在时该部分会**静默跳过**，所以只检出两个仓库也能用。

```bash
```

`set` 的参数顺序：`VERSION CODENAME BUILD_VER`，其余用选项：
`--codename`（小写）、`--debian-ver`、`--variant`、`--variant-id`、`--keep-build-time`。

## 派生规则

| 值 | 来源 |
|---|---|
| `VERSION_TYPE`（Alpha / Beta / Release） | 由 `BUILD_VER` 派生，与 `base-files/debian/rules` 里的既有规则一致：含 `a` → Alpha，含 `b` → Beta，否则 Release |
| `channel`（stable / unstable / canary） | 由阶段映射：`Release→stable`、`Beta→unstable`、`Alpha→canary` |
| `BUILD_ID` | 构建当天日期 `date +%Y%m%d`（由 rules 生成） |
| `BUILD_TIME` | **工具生成版本号时的时间**，格式参考 `lingmo-builder/tools/build_version.sh`：`YY+月+日+时+分钟的十位`，例如 `261014204` |

## 安全特性

* **默认 dry-run**，必须显式 `--write` 才落盘
* 写前自动备份 `*.bak`，写后**回读校验**（`--write` 会再跑一次 `check`）
* **幂等**：内容一致时不做任何改动（重复 `sync` 报 0 处）
* `check` 还会校验"`VERSION_TYPE` 的派生规则是否被人为写死"

## 接 CI

在 `lingmo-builder`（或任意同时检出两个仓库的流水线）里加一步，版本不一致就 fail：

```yaml
      - name: Check system version consistency
        run: |
          git clone --depth 1 https://github.com/LingmoOS/base-files
          python3 lingmo-version/lingmo-version check --root .
```

## 相关仓库

* [base-files](https://github.com/LingmoOS/base-files) —— 版本号的真源（`debian/rules` 的配置块）
* [lingmo-builder](https://github.com/LingmoOS/lingmo-builder) —— ISO 构建器（hook、`build.sh`、`issue`）

## 版本号的四处归属

| 仓库 | 角色 | 谁改 |
|---|---|---|
| [base-files](https://github.com/LingmoOS/base-files) | **唯一源**（`debian/rules` 配置块） | 工具 |
| [lingmo-builder](https://github.com/LingmoOS/lingmo-builder) | ISO 构建器（hook、`build.sh`、`issue`） | 工具 |
| [lingmo-live](https://github.com/LingmoOS/lingmo-live) | 安装器品牌（`branding.desc`） | 工具（该仓库 main 有分支保护，故走 PR） |
| lingmo-installer | 上游 Calamares 引擎，**不含**版本信息 | 无需改 |

注：`lingmo-installer` 仓库里的 `src/branding/default/branding.desc` 是上游占位值
（`Fancy GNU/Linux`），ISO 实际使用 `lingmo-live/calamares/settings.conf` 里指定的
`branding: lingmo`，所以那份不影响发行版。

## 额外的校验（不写文件）

* `lingmo-desktop-base/files/os-version.in`：要求保留 `@@DISTRO_NAME@@`、`@@VERSION@@`、
  `@@BUILD_VER@@` 三个占位符，并且 `SystemName` / `MajorVersion` / `MinorVersion` / `OsBuild`
  不能写成固定值 —— 这份模板会被打包流程与 live hook 替换成 `/etc/os-version`
  （`dtkcore`、系统监视器、设备管理器都读它），写死就会与系统版本脱节。
  用 `--desktop-base <路径>` 指定仓库位置，未检出时自动跳过。
