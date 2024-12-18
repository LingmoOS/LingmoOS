# lingmo 原子更新管理程序
## 前言
lingmo 是一个开放的操作系统，用户可通过多种方式更新系统。若某一次的更新与系统不兼容或是更新过程被意外中断，可能会导致重启后系统无法正常运行，影响用户使用。

lingmo 为了处理此类更新引发的问题，从 v20 开始，设计实现了 AB 双系统分区的方案。即在系统安装时创建两个等大的系统分区，标记为 A 和 B 。 如当前系统是在 A ，系统更新时先将 A 中内容增量同步到 B ，同步完成后，生成对于 B 的引导，然后开始更新 A 。更新完成后重启，若更新存在问题，用户可回退到 B 中的系统。

在实际的使用中，AB 双系统分区的方案面临空间占用多、不支持命令行的问题。

因此需要重新设计一套更新管理程序，实现以下目标：

- 系统状态变更(软件安装、更新、卸载)前备份当前状态，并支持命令行
- 能够回滚到前一个系统状态
- 合并双系统分区为一个系统分区
- 备份的系统存储在数据分区
## 用法
### 编译依赖
- golang
### 运行依赖
- ostree
- mount
### 构建
```shell
make
```
### 安装
```shell
make install
```
### 使用
使用之前需先编写好配置文件，若无特殊要求，配置文件需安装到 `/persistent/osroot/config.json` 。

- 初始化
    - `sudo lingmo-upgrade-manager --action=init`
    - `sudo lingmo-upgrade-manager --action=list`
获取初始化版本号，假定为 `v23.0.0.20220218`

    - `sudo lingmo-upgrade-manager --action=rollback --version=v23.0.0.20220218`
- 提交新版本
    - `sudo update-grub`
    - `sudo lingmo-upgrade-manager --action=commit`
- 回滚到指定版本
需先询问用户是否保存当前系统状态，若需要保存则按照**提交新版本**的步骤进行操作。 假定要回滚到`v23.0.0.20220218`，使用命令：
```shell
sudo lingmo-upgrade-manager --action=rollback --version=v23.0.0.20220218
```
## 设计
### 约束
### 编程语言
- 解释性语言优先选择 shell
- 编译性语言优先选择 `golang`，`C`次之，后期考虑更换为 `rust`
- 当前优先实现功能，所以`golang`优先

- 依赖最小化

    因为需要在 initrd 阶段运行，尽可能的不要引入外部依赖

### 方案
本方案采用 `ostree` 构建系统文件的本地 `repo` ，根据配置文件中的订阅目录，将数据提交到此 `repo` 中。 ~~而系统文件则由每个版本的快照通过 `hardlink` 生成，以节省空间使用。~~

每次系统变更前，都对当前系统状态进行提交，生成新版本。版本回滚时，则使用对应版本的快照重新生成系统文件目录。

详细介绍请见概要设计文档。

## TODO
**更新管理程序**

- [ ] 实现更新程序的单例检测
在执行 `init`、`commit`、`rollback`、`snapshot` 命令时，需检测当前是否有更新程序正在执行这些操作，若存在则直接退出。

- [ ] 回滚前需检查 fstab 中的分区是否都已经挂载
系统文件并不都是在系统分区中，有一部分在数据分区，因此需要检查相关的分区是否都已经挂载。 特别是在 initrd 中，因为在 initrd 中只会挂载系统分区，所以需要检查并挂载需要的分区。

- [ ] `apt/dpkg` 执行前的 `hook` 添加
系统在启动后，在执行 `apt/dpkg` 安装、更新、卸载软件前，需要使用更新程序提交当前系统状态，需要注意的是在一次系统运行周期内(未重启)，更新程序只应执行一次版本提交。

- [ ] 更新结果自检程序
系统状态变更完成重启后，需要对启动阶段每个关键阶段进行检测，记录状态是否正常。若不正常则在用户登录后，提示用户是否回滚。若无法正常进入系统，则标记未失败，重启后自动回滚到上一版本。

- [ ] 版本数据清理功能
默认本地 `repo` 中只保留 **3** 个版本，多余的版本需要清理，清理时版本对应的数据也应清理，过程如下：

  - 查询版本对应的提交： `ostree --repo=/persistent/osroot/os/repo log <version>`

  - 删除版本： `ostree --repo=/persistent/osroot/os/repo refs --delete <version>`
  - 删除数据： `ostree --repo=/persistent/osroot/os/repo prune --delete-commit=<commit>`

  但需保留初始安装版本。

- [ ] DBus 接口
    
    添加必要的 DBus 接口，供控制中心等调用者使用。

- [ ] 确定订阅目录列表
- [ ] 更新/回滚前检查空间是否充足
- [ ] 优化提交/回滚效率
    
    - 可在操作之前计算差异，针对差异数据进行操作

- [ ] 支持从 `/persistent/osroot/snapshot/<version>` 启动系统
- [ ] 配置文件自动生成功能
    
    - 根据不同的分区挂载方式，生成相应的配置文件

- [ ] `/vendor` 合并支持

**initramfs**

- [ ] 实现版本回滚功能

    添加版本获取并回滚。

**安装器**

- [ ] 按照分区要求分区，并创建目录结构
- [ ] 初始化本地 `repo` ，并提交初始数据

**控制中心**

- [ ] 添加版本回滚管理界面

**V20 兼容**

- [ ] `AB` 分区处理

### 远期目标

**更新管理程序**

- [ ] `OTA` 更新支持
**文件恢复**

从 `snapshot` 恢复，若 `snapshot` 也被损坏，则从 `repo` 中恢复，`repo` 也被损坏则放弃。

- [ ] 文件篡改修复
- [ ] 文件丢失修复

**initramfs 模块化**
- [ ] 确定基础 img
- [ ] 确定模块格式及生成工具
- [ ] 确定模块加载方式