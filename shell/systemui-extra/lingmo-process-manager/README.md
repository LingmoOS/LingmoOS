# lingmo-process-manager介绍
## 简介
`lingmo-process-manager`即分级冻结，主要负责将应用进行分组分级管理，针对应用的分类和状态分别进行不同程度的系统资源限制。在系统高负载的情况下能够保证系统和当前用户正在操作的应用的流畅度，同时还能进一步降低系统功耗，提升系统续航时间。
此外，负责打开桌面应用，对外提供了应用打开的统一入口，在平板模式下实现了应用单实例功能。

## 编译依赖
- `libglib2.0-dev`
- `qtbase5-dev`
- `qttools5-dev`
- `libkf5config-dev`
- `libkf5windowsystem-dev`
- `libkf5kio-dev`
- `liblingmo-log4qt-dev`
- `libjsoncpp-dev`
- `pkg-config`
- `libgsettings-qt-dev`
- `libprocps-dev`
- `libcgroup-dev`
- `libkf5wayland-dev`
- `liblingmosdk-waylandhelper-dev`
- `liblingmosdk-datacollect-dev`
- `libx11-dev`

## 编译
```shell
cmake -S . -B build
cd build
make
```
## 安装
```shell
sudo make install
```

## Dbus接口 

### 打开应用的接口
- **type**:         session D-Bus
- **service**:      com.lingmo.ProcessManager
- **path**:         /com/lingmo/ProcessManager/AppLauncher
- **interface**:    com.lingmo.ProcessManager.AppLauncher
#### 方法       
1. **LaunchApp**(in 's' desktopFile)
    打开desktopFile文件对应的应用程序
   - desktopFile: 应用对应的desktop文件，使用绝对路径
2. **LaunchAppWithArguments**(in 's' desktopFile, in 'as' args)
    打开desktopFile文件对应的应用程序并且传入参数args
   - desktopFile: 应用对应的desktop文件，使用绝对路径
   - args: 启动参数。比如文本编辑器打开文件，参数需要传文件的路径 
3. **LaunchDefaultAppWithUrl**(in 's' url)
    使用默认应用打开url
   - url: 资源对应的url，例如本地文件 file:/home/lingmo/readme.txt，网址https://www.baidu.com
4. **GetAvailableAppListForFile**(in 's' fileName, out 'as' appList)
    获取支持打开指定文件类型的应用列表
   - fileName: 要获取的应用列表支持的文件，绝对路径
   - appList: 返回支持fileName的应用列表
5. **RunCommand**(in 's' command)
    执行命令command
   - command: 要执行的命令
6. **GetDesktopFileByPid**(in 'i' pid, out 's' desktopFile)
    获取pid对应的desktop文件
   - pid: 要获取的desktop文件对应的pid
   - desktopFile: 返回pid对应的desktop文件的绝对路径，未找到时返回空
#### 信号
1. **AppLaunched**(out 's' id)
    应用启动成功之后发出该信号
   - id：应用绝对路径的desktop文件。应用不包含desktop文件时，使用启动参cmdline
### 进程管理相关接口
- **type**:         session D-Bus
- **service**:      com.lingmo.ProcessManager
- **path**:         /com/lingmo/ProcessManager
- **interface**:    com.lingmo.ProcessManager
#### 方法
1. ThawProcess(in 'i' pid)
    唤醒进程，解除冻结状态
  - pid：需要唤醒的进程
2. ThawFrozenProcesses()
    唤醒所有冻结的进程

## 其他组件的交互
+ 所有需要启动第三方桌面应用或系统软件均需要通过应用管理的接口进行启动，比如开始菜单，侧边栏，任务栏，桌面和文件管理器等。

## 联系我们
- `https://gitee.com/lingmo/lingmo-process-manager`
