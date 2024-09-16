# AM重构计划

目前对AM应该有怎么样的特性、以及其负责的边界，
已经在[AM-ICU房间][1]中经过了详细的讨论。
已经到了可以着手进行初步的设计和评审工作的阶段。

[1]: https://matrix.to/#/!WBkiaAZKeeKzerJEZQ:deepin.org?via=deepin.org&via=matrix.org&via=mozilla.org

## 背景

由于目前老AM项目中尚有dock和launcher项目的逻辑，
所以在重构时，
需要同步地将dock和launcher中属于后台服务部分的逻辑写回对应项目中去。
该部分工作会与AM的重构工作同步进行，由负责dock和launcher的研发人员完成。
这两个任务与AM的重构工作不应该产生强的依赖关系。

基于以上背景，老的AM并不会直接被移除，我们将新建一个项目完成新AM的编写。
完成其主要功能以后再将老AM中的启动应用功能移除，
并将dock和launcher中启动应用的部分逻辑切换到新AM提供的DBus接口。

老AM中launcher和dock的后端逻辑不会进行任何更改，
直到等待launcher和dock已经不再依赖老AM后，
彻底将旧AM从系统中移除。

## AM的主要功能

1. DDE的desktop文件管理器。

   所有需要获取Desktop文件的进程，都应该通过AM来进行这个操作。
   AM会提供两套接口完成对desktop文件的访问以及管理：

   1. DBus
   2. fuse 用于完全实现XDG兼容。\[暂不实现\]

   这两套接口都需要考虑接口的调用者是从容器中访问的情况。

2. DDE的应用执行者。

   需要提供DBus接口并更新dde-open。

## 工作记录

由参与AM重构计划的研发人员完成的主要的工作有以下几方面，
可以拆分为几个阶段来完成：

- 准备
  - [x] 完成AM的DBus API设计文档（@black-desk）；

    本次AM共计需要完成5个DBus接口:

    1. [`org.desktopspec.ApplicationManager1`](../api/dbus/org.desktopspec.ApplicationManager1.xml)
    2. [`org.desktopspec.ApplicationManager1.Application`](../api/dbus/org.desktopspec.ApplicationManager1.Application.xml)
    3. [`org.desktopspec.ApplicationManager1.Instance`](../api/dbus/org.desktopspec.ApplicationManager1.Instance.xml)
    4. [`org.desktopspec.JobManager1`](../api/dbus/org.desktopspec.JobManager1.xml)
    5. [`org.desktopspec.JobManager1.Job`](../api/dbus/org.desktopspec.JobManager1.Job.xml)

  - [x] 以上接口设计需要通过评审。DEADLINE: 23.7.5

- [x] 项目框架搭建 23.7.21
  - [x] Job机制的基本实现;

- [ ] 主要功能开发
  - [x] 应用列表；23.7.28
  - [x] 启动应用；23.7.28
  - [x] 从PID获取AppID；23.7.28
  - [x] 正在运行的应用列表；23.8.4
  - [ ] 崩溃后恢复现场；23.8.4

- [ ] launcher/dock/xdg-open迁移 23.8.11
  
  更改launcher和dock中启动应用部分的代码，使用新AM来实现。
  并删除老AM中启动应用相关的代码。

- [ ] 测试工作 

  进行

  - [ ] dock
  - [ ] launcher
  - [ ] dde-open/xdg-open
  - [ ] 文件管理器
  - [ ] 浏览器

  等桌面组件的基础功能测试，bug修复。

- [ ] hook功能开发\[暂不实现\]

  该功能的开发主要是为了防止应用从配置好的cgroup中逃逸。

  1. 完成App运行前hook插件体系的实现；
  2. 重要插件实现：

     - [ ] ~~cgroup锁定插件的实现；(@black-desk) ~~

       ~~<https://github.com/black-desk/djail>~~

       已经没有必要做这个工作了， systemd-v254已经解决了用户级服务无法锁定cgroupfs的问题。

     - [ ] dbus代理插件；(@black-desk) 

       使用flatpak提供的xdg-dbus-proxy实现一个不可交互的dbus代理，
       目前主要用于屏蔽systemd用户实例提供的cgroup移动功能。

       使用一个具有特权的二进制创建新的mount namepsace，
       通过挂载将proxy提供的socket bind到对应位置。
