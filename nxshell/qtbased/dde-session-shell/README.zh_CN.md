# DDE Session Shell

DDE session shell 提供两个应用程序：dde-lock 和 lightdm-deepin-greeter，dde-lock提供锁屏界面，lightdm-deepin-greeter提供登录界面。

## 依赖
请查看“debian/control”文件中提供的“Depends”。

### 编译依赖
请查看“debian/control”文件中提供的“Build-Depends”。

## 安装

### 构建过程

1. 确保已经安装了所有的编译依赖
```bash
sudo apt build-dep dde-session-shell
```

2. 构建
```bash
mkdir build
cd build
cmake ..
make
```

3. 安装
```bash
sudo make install
```

4. debian/rules

```makefile
override_dh_auto_configure:
	dh_auto_configure -- \
		-DWAIT_DEEPIN_ACCOUNTS_SERVICE=1
```
lightdm-deepin-greeter 强依赖 com.deepin.daemon.Accounts 服务 (由dde-system-daemon创建)。
我们使用 WAIT_DEEPIN_ACCOUNTS_SERVICE 来判断 lightdm-deepin-greeter 的启动是否等待dde-system-daemon，如果桌面环境没有启动dde-system-daemon，请设置 WAIT_DEEPIN_ACCOUNTS_SERVICE 为 0，否则 deepin-lightdm-greeter 会卡住。

## 帮助
任何使用问题都可以通过以下方式寻求帮助：

* [Matrix](https://matrix.to/#/#deepin-community:matrix.org)
* [WiKi](https://wiki.deepin.org)
* [官方论坛](https://bbs.deepin.org)
* [开发者中心](https://github.com/linuxdeepin/developer-center/issues) 

## 贡献指南

我们鼓励您报告问题并做出更改

- [开发者代码贡献指南](https://github.com/linuxdeepin/developer-center/wiki/Contribution-Guidelines-for-Developers) 

## 开源许可证
DDE session shell 在 [GPL-3.0-or-later](LICENSE) 下发布。
