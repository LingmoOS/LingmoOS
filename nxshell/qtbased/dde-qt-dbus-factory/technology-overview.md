
# dde-qt-dbus-factory

`dde-qt-dbus-factory` 主要想解决的问题有：

1. 所有的 DBus 接口都是用 xml 描述的，将生成的 `.h`、`.cpp` 文件保存在源码树中没有太大意义。
2. 目前 Qt 自己的 xml 生成 cpp 的转换工具只提供了很基本的功能，像 PropertyChanged 信号这种很常用的功能都没有提供。
3. 目前 Qt 的生成工具在对 Property 进行读取、设置的时候，是强制阻塞的，这种设计有可能会导致 DBus 卡死。
4. 我们对 DBus 接口有更多的二进制兼容要求及效率、队列、异步通信要求。

## 解决方案

目前本项目的主要研究工作从几个方面入手：一个是改进原有的 `xml2cpp` 的转换工具，使它生成的类具有更好的结构，同时实现了二进制兼容、消息队列、PropertyChanged 信号等功能。另一个是参考 `Meego` 的部分开源代码的思路，开发了利用 DBus 自己的 `Property.Get` 和 `Propergy.Set` 来进行属性的设置与获取，这样可以做到异步的属性访问控制。

这两部分是需要相互配合的。

# 整体设计

整体上，项目分为三块

1. 一个经过二次开发的 `qdbusxml2cpp`，它可以生成出与 Qt 原有工具完全不同的 cpp 类来，并具有多种新功能。
    - 添加了 namespace，防止类冲突
    - 实现了 d-ptr 结构，保持更好的二进制兼容性。
    - 优化了自定义类型的支持
2. 一个利用本项目编译出来的 `qdbusxml2cpp` 工具与 DBus 接口描述的 xml 文件生成具体的接口类的工具 `generate_code.py`。
3. 一个经过适当修改的 `DBusExtendedAbstractInterface`，在这里实现了对 Property 的异步访问与设置，也添加了对一些自启动服务的自动 launch 及监控功能。

## qdbusxml2cpp

参考 `qdbusxml2cpp` 目录中的代码，除改变了输出格式，添加了一些 namespace 之类的之外，和原来的工作流程没有什么区别。

## generate_code.py

一个利用编译出来的 `qdbusxml2cpp`，将 xml 描述的 DBus 接口文件转换成 cpp 接口类。

