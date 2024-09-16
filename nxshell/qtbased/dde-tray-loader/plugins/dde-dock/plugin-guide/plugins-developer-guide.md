# 开发任务栏插件

在本文中，你将了解任务栏插件的 API 说明，并跟随示例开发一个简易的任务栏插件。

## 插件的工作原理

插件是在不需要改动并重新编译主程序本身的情况下扩展主程序功能的一种机制。dde-dock 插件是根据 Qt 插件标准所开发的共享库文件(.so)，可以通过实现 Qt 的插件标准和 dde-dock 提供的接口来完成 dde-dock 的功能扩展。可以通过以下链接查看关于 Qt 插件更详细的介绍：

- https://wiki.qt.io/Plugins
- https://doc.qt.io/qt-5/plugins-howto.html

## dde-dock 插件加载流程

在 dde-dock 启动时会启动一个线程去检测目录 `/usr/lib/dde-dock/plugins` 下的所有文件，并检测是否是一个正常的动态库文件，如果是则尝试加载。尝试加载即检测库文件的元数据，插件的元数据定义在一个 JSON 文件中，这个后文会介绍。如果元数据检测通过就开始检查插件是否实现了 dde-dock 指定的接口，这一步也通过之后就会开始初始化插件，获取插件提供的控件，进而将控件显示在任务栏上。

## 接口列表

这里先介绍 dde-dock 都提供了哪些接口，可作为一个手册查看。注意，为 dde-dock 编写插件并不是要实现所有接口，这些接口提供了 dde-dock 允许的各种可能的功能，插件开发者可以根据需求实现自己需要的接口。后续的插件示例也会用到这里列出的部分接口。接口定义的文件一般在系统如下位置：

> /usr/include/dde-dock/pluginproxyinterface.h
> /usr/include/dde-dock/pluginsiteminterface.h
> /usr/include/dde-dock/constants.h

只有标明必须实现的接口才是必须要由插件开发者实现的接口，对于其他接口，如果不需要对应功能可不实现，下面详细介绍一下 PluginItemInterface 和 PluginProxyInterface 两个类中包含的接口。

### PluginItemInterface

PluginsItemInterface 中定义的接口除了 displayMode 和 position（历史遗留），从插件的角度来看都是被动的，只能等待被任务栏的插件机制调用，其接口如下表所示。

| 名称                 | 简介                                                                       | 必须实现 |
| -------------------- | -------------------------------------------------------------------------- | -------- |
| pluginName           | 返回插件名称，用于在 dde-dock 内部管理插件时使用                           | 是       |
| pluginDisplayName    | 返回插件名称，用于在界面上显示                                             | 否       |
| init                 | 插件初始化入口函数，参数 proxyInter 可认为是主程序本身                     | 是       |
| itemWidget           | 返回插件主控件，用于显示在 dde-dock 面板上                                 | 是       |
| itemTipsWidget       | 返回鼠标悬浮在插件主控件上时显示的提示框控件                               | 否       |
| itemPopupApplet      | 返回鼠标左键点击插件主控件后弹出的控件                                     | 否       |
| itemCommand          | 返回鼠标左键点击插件主控件后要执行的命令数据                               | 否       |
| itemContextMenu      | 返回鼠标右键点击插件主控件后要显示的菜单数据                               | 否       |
| invokedMenuItem      | 菜单项被点击后的回调函数                                                   | 否       |
| itemSortKey          | 返回插件主控件的排序位置                                                   | 否       |
| setSortKey           | 重新设置主控件新的排序位置（用户拖动了插件控件后）                         | 否       |
| itemAllowContainer   | 返回插件控件是否允许被收纳                                                 | 否       |
| itemIsInContainer    | 返回插件是否处于收纳模式（仅在 itemAllowContainer 为 true 时有作用）       | 否       |
| setItemIsInContainer | 更新插件是否处于收纳模式的状态（仅在 itemAllowContainer 主 true 时有作用） | 否       |
| pluginIsAllowDisable | 返回插件是否允许被禁用（默认不允许被禁用）                                 | 否       |
| pluginIsDisable      | 返回插件当前是否处于被禁用状态                                             | 否       |
| pluginStateSwitched  | 当插件的禁用状态被用户改变时此接口被调用                                   | 否       |
| displayModeChanged   | dde-dock 显示模式发生改变时此接口被调用                                    | 否       |
| positionChanged      | dde-dock 位置变化时时此接口被调用                                          | 否       |
| refreshIcon          | 当插件控件的图标需要更新时此接口被调用                                     | 否       |
| displayMode          | 用于插件主动获取 dde-dock 当前的显示模式                                   | 否       |
| position             | 用于插件主动获取 dde-dock 当前的位置                                       | 否       |

### PluginProxyInterface

由于上面介绍的接口对于插件来说基本都是被动的，即插件本身无法确定这些接口什么时候会被调用，很明显对于插件机制来说这是不完整的，因此便有了 PluginProxyInterface。它定义了一些让插件主动调用以控制 dde-dock 的一些行为的接口。

PluginProxyInterface 的具体实例可以认为是抽象了的 dde-dock 主程序，或者是 dde-dock 中所有插件的管理员，这个实例将会通过 PluginsItemInterface 中的 init 接口传递给插件，因此在上述 init 接口中总是会先把这个传入的对象保存起来以供后续使用。PluginProxyInterface 类中的接口如下表所示。

| 名称                        | 简介                                                                                                                               |
| --------------------------- | ---------------------------------------------------------------------------------------------------------------------------------- |
| itemAdded                   | 向 dde-dock 添加新的主控件（一个插件可以添加多个主控件，它们之间使用 itemKey 来区分）                                              |
| itemUpdate                  | 通知 dde-dock 有主控件需要更新                                                                                                     |
| itemRemoved                 | 从 dde-dock 移除主控件                                                                                                             |
| requestWindowAutoHide       | 设置 dde-dock 是否允许隐藏，通常用在任务栏被设置为智能隐藏或始终隐藏而插件又需要让 dde-dock 保持显示状态来显示一些重要信息的场景下 |
| requestRefreshWindowVisible | 通知 dde-dock 更新隐藏状态                                                                                                         |
| requestSetAppletVisible     | 通知 dde-dock 显示或隐藏插件的弹出面板（鼠标左键点击后弹出的控件）                                                                 |
| saveValue                   | 统一的配置保存函数                                                                                                                 |
| getValue                    | 统一的配置读取函数                                                                                                                 |

## 构建一个 dde-dock 插件

接下来将介绍一个简单的 dde-dock 插件的开发过程，插件开发者可据此熟悉 dde-dock 插件开发的步骤，以便创造出更多具有丰富功能的插件。

### 预期功能

首先来确定下这个插件所需要的功能：

- 实时显示 home 分区(即 /home 目录对应的分区)可使用的剩余空间大小百分比。
- 允许禁用插件。
- 鼠标指针悬浮在插件上时显示 home 分区总容量和可用容量。
- 鼠标左键单击插件时打开一个提示框，显示关于 home 分区更详细的信息。
- 鼠标右键点击插件显示一个菜单，用于刷新缓存和启动 gparted 程序（GParted 是一个免费的分区编辑器，用于图形化管理磁盘分区）。

### 安装依赖

下面以 Qt + cmake 为例进行说明，以统信 UOS 1060 环境为基础，安装如下的包：

- dde-dock-dev
- cmake
- qtbase5-dev-tools
- pkg-config

### 插件基础框架

创建必需的项目目录与文件，插件名称叫做 `home-monitor`，目录结构如下：

> home-monitor
> ├── home-monitor.json
> ├── homemonitorplugin.cpp
> ├── homemonitorplugin.h
> └── CMakeLists.txt

接着来依次分析各个文件的作用。

#### cmake 配置文件

`CMakeLists.txt`是 cmake 命令要读取的配置文件，用于管理整个项目的源文件，依赖，构建等等，其内容如下：

> 以 `#`开头的行是注释，用于介绍相关命令，对创建一份新的 CMakeLists.txt 文件有帮助，目前可以简单地了解。

```cmake
# 学习 cmake 时建议直接从命令列表作为入口，查阅命令的网站：
# https://cmake.org/cmake/help/latest/manual/cmake-commands.7.html
# 下面是完整的文档入口的链接：
# https://cmake.org/cmake/help/latest/

# 设置运行此配置所需的 cmake 的最低版本
cmake_minimum_required(VERSION 3.11)

# 使用 set 命令设置一个变量
set(PLUGIN_NAME "home-monitor")

# 设置项目名称
project(${PLUGIN_NAME})

# 启用 qt moc 的支持
set(CMAKE_AUTOMOC ON)
# 启用 qrc 资源文件的支持
set(CMAKE_AUTORCC ON)

# 指定所有源码文件
# 使用了 cmake 的 file 命令，递归查找项目目录下所有头文件和源码文件
# 并将结果放入 SRCS 变量中，方便后续取用
file(GLOB_RECURSE SRCS "*.h" "*.cpp")

# 指定要用到的库
# 使用了 cmake 的 find_package 命令，查找库 Qt5Widgets 等，
# REQUIRED 参数表示如果没有找到则报错
# find_package 命令在找到并加载指定的库之后会设置一些变量
# 常用的有：
# <库名>_FOUND          是否找到（Qt5Widgets_FOUND）
# <库名>_DIR            在哪个目录下找到的（Qt5Widgets_DIR）
# <库名>_INCLUDE_DIRS   有哪些头文件目录（Qt5Widgets_INCLUDE_DIRS）
# <库名>_LIBRARIES      有哪些库文件（Qt5Widgets_LIBRARIES）
find_package(Qt5Widgets REQUIRED)
find_package(DtkWidget REQUIRED)

# find_package 命令还可以用来加载 cmake 的功能模块
# 并不是所有的库都直接支持 cmake 查找的，但大部分都支持了 pkg-config 这个标准，
# 因此 cmake 提供了间接加载库的模块：FindPkgConfig， 下面这行命令表示加载 FindPkgConfig 模块，
# 这个 cmake 模块提供了额外的基于 “pkg-config” 加载库的能力
# 执行下面的命令后后会设置如下变量，不过一般用不到：
# PKG_CONFIG_FOUND            pkg-config 可执行文件是否找到了
# PKG_CONFIG_EXECUTABLE       pkg-config 可执行文件的路径
# PKG_CONFIG_VERSION_STRING   pkg-config 的版本信息
find_package(PkgConfig REQUIRED)

# 加载 FindPkgConfig 模块后就可以使用 pkg_check_modules 命令加载需要的库
# pkg_check_modules 命令是由 FindPkgConfig 模块提供的，因此要使用这个命令必须先加载 FindPkgConfig 模块。
# 执行 pkg_check_modules 命令加载库也会设置一些类似执行 find_package 加载库后设置的变量：
# DdeDockInterface_FOUND
# DdeDockInterface_INCLUDE_DIRS
# DdeDockInterface_LIBRARIES
# 还有有另外的一些变量以及更灵活的用法，比如一次性查找多个库，这些请自行查找 cmake 文档学习。
pkg_check_modules(DdeDockInterface REQUIRED dde-dock)

# add_definitions 命令用于声明/定义一些编译/预处理参数
# 根据 cmake 文档描述此命令已经有另外几个功能划分的更为细致的命令所取代，具体请查阅文档
# 在我们这里的例子应该使用较新的 add_compile_definitions 命令，不过为了保持与 dock 已有插件一致，
# 暂时仍然使用 add_definitions，add_definitions 的语法很简单就是直接写要定义的 flag 并在前面加上 "-D" 即可
# 括号中的 ${QT_DEFINITIONS} 变量会在执行 cmake 时展开为它的值，这个变量属于历史遗留，应该是在 qt3/qt4 时有用，
# 基于 qt5 或更高版本的新插件不必使用此变量。要查看 qt5 的库定义了哪些变量应该查看变量：${Qt5Widgets_DEFINITIONS}
add_definitions("${QT_DEFINITIONS} -DQT_PLUGIN")

# 新增一个编译目标
# 这里使用命令 add_library 来表示本项目要生成一个库文件目标，
# 类似的还有命令 add_executable 添加一个可执行二进制目标，甚至 add_custom_target(使用较少) 添加自定义目标
# SHARED 表示生成的库应该是动态库，
# 变量 ${PLUGIN_NAME} 和 ${SRCS} 都是前面处理好的，
# 另外 qrc 资源文件也应该追加在后面以编译进目标中。
add_library(${PLUGIN_NAME} SHARED ${SRCS} home-monitor.qrc)

# 设置目标的生成位置，这里表示生成在执行 make 的目录,
# 另外还有很多可用于设置的属性，可查阅 cmake 文档。
set_target_properties(${PLUGIN_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ./)

# 设置目标要使用的 include 目录，即头文件目录
# 变量 ${DtkWidget_INCLUDE_DIRS} 是在前面执行 find_package 命令时引入的
# 当出现编译失败提示找不到某些库的头文件时应该检查此处是否将所有需要的头文件都包含了
target_include_directories(${PLUGIN_NAME} PUBLIC
    ${Qt5Widgets_INCLUDE_DIRS}
    ${DtkWidget_INCLUDE_DIRS}
    ${DdeDockInterface_INCLUDE_DIRS}
)

# 设置目标要使用的链接库
# 变量 ${DtkWidget_LIBRARIES} 和 ${Qt5Widgets_LIBRARIES} 是在前面执行 find_package 命令时引入的
# 当出现运行时错误提示某些符号没有定义时应该检查此处是否将所有用的库都写在了这里
target_link_libraries(${PLUGIN_NAME} PRIVATE
    ${Qt5Widgets_LIBRARIES}
    ${DtkWidget_LIBRARIES}
    ${DdeDockInterface_LIBRARIES}
)

# 设置安装路径的前缀(默认为"/usr/local")
set(CMAKE_INSTALL_PREFIX "/usr")

# 设置执行 make install 时哪个目标应该被 install 到哪个位置
install(TARGETS ${PLUGIN_NAME} LIBRARY DESTINATION lib/dde-dock/plugins)
```

#### 元数据文件

`home-monitor.json`文件是插件的元数据文件，指明了当前插件所使用的 dde-dock 的接口版本。dde-dock 在加载此插件时，会检测自己的接口版本是否与插件的接口版本一致，当双方的接口版本不一致或者不兼容时，dde-dock 为了安全将阻止加载对应的插件。另外，元数据文件是在源码中使用特定的宏加载到插件中的。在 dde-dock 内建的插件代码中，可以找到当前具体的接口版本，在本书编写时最新 的版本是 1.2。代码如下:

```json
{
  "api": "1.2"
}
```

另外还可以指定 D-Bus 服务(可选的，且仅支持 session D-Bus)，dde-dock 在加载插件时会检查此插件所依赖的 D-Bus 服务，如果服务没有启动则不会初始化这个插件，直到服务启动。下面的代码表示依赖 dbus 地址为“com.deepin.daemon.Network”的 D-Bus 服务。代码如下：

```json
{
  "api": "1.2",
  "depends-daemon-dbus-service": "com.deepin.daemon.Network"
}
```

#### 插件核心类

`homemonitorplugin.h` 声明了类 `HomeMonitorPlugin`，它继承（实现）了前面提到的 `PluginItemInterface`，这代表它是一个实现了 dde-dock 接口的插件。

下面是最小化实现了 dde-dock 接口的插件的源码，只实现了必须实现的接口。注意:下文的代码只是为了简述开发插件的主要过程，详细的示例代码可查看 home-monitor 目录下的内容。

```c++
#ifndef HOMEMONITORPLUGIN_H
#define HOMEMONITORPLUGIN_H

#include <dde-dock/pluginsiteminterface.h>

#include <QObject>

class HomeMonitorPlugin : public QObject, PluginsItemInterface
{
    Q_OBJECT
    // 声明实现了的接口
    Q_INTERFACES(PluginsItemInterface)
    // 插件元数据
    Q_PLUGIN_METADATA(IID "com.deepin.dock.PluginsItemInterface" FILE "home-monitor.json")

public:
    explicit HomeMonitorPlugin(QObject *parent = nullptr);

    // 返回插件的名称，必须是唯一值，不可以和其它插件冲突
    const QString pluginName() const override;

    // 插件初始化函数
    void init(PluginProxyInterface *proxyInter) override;

    // 返回插件的 widget
    QWidget *itemWidget(const QString &itemKey) override;
};

#endif // HOMEMONITORPLUGIN_H
```

`homemonitorplugin.cpp` 中包含对应接口的实现

```c++
#include "homemonitorplugin.h"

#include <QLabel>

HomeMonitorPlugin::HomeMonitorPlugin(QObject *parent)
    : QObject(parent)
{

}

const QString HomeMonitorPlugin::pluginName() const
{
    return QStringLiteral("home-monitor");
}

void HomeMonitorPlugin::init(PluginProxyInterface *proxyInter)
{
    m_proxyInter = proxyInter;

    m_proxyInter->itemAdded(this, pluginName());
}

QWidget *HomeMonitorPlugin::itemWidget(const QString &itemKey)
{
    Q_UNUSED(itemKey);

    // 返回一个图标展示在任务栏上
    static QLabel* item = nullptr;
    if (!item) {
        item = new QLabel();
        item->setFixedSize(20, 20);
        item->setPixmap(QIcon::fromTheme("deepin-diskmanager").pixmap(QSize(20, 20))); // QIcon::fromTheme会从主题里面查找图标
    }

    return item;
}
```

### 测试插件加载

插件的基本结构搭建好之后，应该测试这个插件能否被 dde-dock 正确加载，这时候如果测试出有问题可以及时处理。

#### 从源码构建

为了不污染源码目录，推荐在源码目录中创建 `build` 目录用于构建：

```sh
cd home-monitor

mkdir build

cd build

cmake ..

make -j4
```

#### 安装

执行下面的命令即可将插件安装到系统中，也是 CMakeList.txt 文件指定的安装位置：

```sh
sudo make install
```

可以看到有 `home-monitor.so`文件被安装在了 dde-dock 的插件目录。

```sh
install -m 755 -p ./home-monitor/libhome-monitor.so /usr/lib/dde-dock/plugins/libhome-monitor.so
```

或者将 so 文件放在~/.local/lib/dde-dock/plugins/目录下，dock 会加载用户本地的插件。

```sh
install -m 755 -p ./home-monitor/libhome-monitor.so ~/.local/lib/dde-dock/plugins/libhome-monitor.so
```

#### 测试加载

执行 `pkill dde-dock` 命令“杀死”任务栏，看门狗会自动启动任务栏，此时任务栏上会多了一个磁盘样式的插件。此时这个插件只能展示一个简单的图标，没有任何其它的功能，下面将增加弹窗、右键菜单、提示等功能，来完成我们的预期功能。

### 实现预期功能

#### 创建插件主控件

创建新文件 informationwidget.h 和 informationwidget.cpp，用于创建控件类：InformationWidget，这个控件用于显示在 dde-dock 上。

此时的目录结构为：

> home-monitor
> ├── build/
> ├── home-monitor.json
> ├── homemonitorplugin.cpp
> ├── homemonitorplugin.h
> ├── informationwidget.cpp
> ├── informationwidget.h
> └── CMakeLists.txt

informationwidget.h 文件内容如下：

```c++
#ifndef INFORMATIONWIDGET_H
#define INFORMATIONWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QTimer>
#include <QStorageInfo>

class InformationWidget : public QWidget
{
    Q_OBJECT

public:
    explicit InformationWidget(QWidget *parent = nullptr);

    inline QStorageInfo * storageInfo() { return m_storageInfo; }

private slots:
    // 用于更新数据的槽函数
    void refreshInfo();

private:
    // 真正的数据显示在这个 Label 上
    QLabel *m_infoLabel;
    // 处理时间间隔的计时器
    QTimer *m_refreshTimer;
    // 分区数据的来源
    QStorageInfo *m_storageInfo;
};

#endif // INFORMATIONWIDGET_H
```

informationwidget.cpp 文件包含了对类 InformationWidget 的实现，内容如下：

```c++
#include "informationwidget.h"

#include <QVBoxLayout>
#include <QTimer>
#include <QDebug>

InformationWidget::InformationWidget(QWidget *parent)
    : QWidget(parent)
    , m_infoLabel(new QLabel)
    , m_refreshTimer(new QTimer(this))
    // 使用 "/home" 初始化 QStorageInfo
    // 如果 "/home" 没有挂载到一个单独的分区上，QStorageInfo 收集的数据将会是根分区的
    , m_storageInfo(new QStorageInfo("/home"))
{
    m_infoLabel->setStyleSheet("QLabel {"
                               "color: white;"
                               "}");
    m_infoLabel->setAlignment(Qt::AlignCenter);

    QVBoxLayout *centralLayout = new QVBoxLayout;
    centralLayout->addWidget(m_infoLabel);
    centralLayout->setSpacing(0);
    centralLayout->setMargin(0);

    setLayout(centralLayout);

    // 连接 Timer 超时的信号到更新数据的槽上
    connect(m_refreshTimer, &QTimer::timeout, this, &InformationWidget::refreshInfo);

    // 设置 Timer 超时为 10s，即每 10s 更新一次控件上的数据，并启动这个定时器
    m_refreshTimer->start(10000);

    refreshInfo();
}

void InformationWidget::refreshInfo()
{
    // 获取分区总容量
    const double total = m_storageInfo->bytesTotal();
    // 获取可用总容量
    const double available = m_storageInfo->bytesAvailable();
    // 得到可用百分比
    const int percent = qRound(available / total * 100);

    // 更新内容
    m_infoLabel->setText(QString("Home:\n%1\%").arg(percent));
}
```

现在主控件类已经完成了，回到插件的核心类，将主控件类添加到核心类中。

在 `homemonitorplugin.h` 中相应位置添加成员声明：

```c++
#include "informationwidget.h"

class HomeMonitorPlugin : public QObject, PluginsItemInterface
{
private:
    InformationWidget *m_pluginWidget;
};
```

然后在 `homemonitorplugin.cpp` 中将添加成员的初始化，比如在 `init` 接口中初始化：

```c++
void HomeMonitorPlugin::init(PluginProxyInterface *proxyInter)
{
    m_proxyInter = proxyInter;

    m_pluginWidget = new InformationWidget;
}
```

#### 添加主控件到 dde-dock 面板上

在插件核心类的 init 方法中，获取到了 PluginProxyInterface 对象，调用此对象的 itemAdded 接口即可实现向 dde-dock 面板添加项目。第二个参数为 QString 类型，代表了本插件所提供的主控件的 ID，当一个插件提供多个主控件时，不同主控件之间要保证 ID 唯一。代码如下:

```c++
void HomeMonitorPlugin::init(PluginProxyInterface *proxyInter)
{
    m_proxyInter = proxyInter;

    m_pluginWidget = new InformationWidget;

    m_proxyInter->itemAdded(this, pluginName());
}
```

调用 itemAdded 之后，dde-dock 会在合适的时机调用插件的 itemWidget 接口以获取需要显示的控件。如果插件提供多个主控件到 dde-dock 上，那么插件核心类应该在 itemWidget 接口中分析参数 itemKey，并返回与之对应的控件对象，当插件只有一个可显示项目时，itemKey 可以忽略(但不建议忽略)。代码如下:

```c++
QWidget *HomeMonitorPlugin::itemWidget(const QString &itemKey)
{
    Q_UNUSED(itemKey);

    return m_pluginWidget;
}
```

编译、安装、重启 dde-dock，就可以看到主控件在 dde-dock 面板上出现了，如下图所示：

![central-widget](images/central-widget.png)

#### 支持禁用插件

与插件禁用和启用相关的接口有如下四个：

- pluginIsAllowDisable
- pluginIsDisable
- pluginStateSwitched
- pluginDisplayName

核心类头文件中增加函数声明：

```c++
bool pluginIsAllowDisable() override;
bool pluginIsDisable() override;
void pluginStateSwitched() override;
const QString pluginDisplayName() const override;
```

核心类源文件中增加函数实现：

```c++
bool HomeMonitorPlugin::pluginIsAllowDisable()
{
    // 告诉 dde-dock 本插件允许禁用
    return true;
}

bool HomeMonitorPlugin::pluginIsDisable()
{
    // 第二个参数 “disabled” 表示存储这个值的键（所有配置都是以键值对的方式存储的）
    // 第三个参数表示默认值，即默认不禁用
    return m_proxyInter->getValue(this, "disabled", false).toBool();
}

void HomeMonitorPlugin::pluginStateSwitched()
{
    // 获取当前禁用状态的反值作为新的状态值
    const bool disabledNew = !pluginIsDisable();
    // 存储新的状态值
    m_proxyInter->saveValue(this, "disabled", disabledNew);

    // 根据新的禁用状态值处理主控件的加载和卸载
    if (disabledNew) {
        m_proxyInter->itemRemoved(this, pluginName());
    } else {
        m_proxyInter->itemAdded(this, pluginName());
    }
}

const QString HomeMonitorPlugin::pluginDisplayName() const
{
    // 返回插件的名称，这个名称可以是多语言的
    return QString(tr("Home Monitor"));
}
```

此时就会引入一个新的问题，插件允许被禁用，那么在 dde-dock 启动时，插件有可能处于禁用状态，那么在初始化插件时就不能直接将主控件添加到 dde-dock 中，而是应该判断当前是否是禁用状态，修改接口 init 的实现：

```c++
void HomeMonitorPlugin::init(PluginProxyInterface *proxyInter)
{
    m_proxyInter = proxyInter;

    m_pluginWidget = new InformationWidget;

    // 如果插件没有被禁用则在初始化插件时才添加主控件到面板上
    if (!pluginIsDisable()) {
        m_proxyInter->itemAdded(this, pluginName());
    }
}
```

#### 支持 hover tip

“hover tip” 就是鼠标指针移动到插件主控件上并悬停一小段时间后弹出的一个提示框，可以用于 显示一些状态信息等，当然具体用来显示什么内容则完全由插件开发者决定，要实现这个功能需要接口 itemTipsWidget。首先在插件核心类中添加一个文本控件作为 tip 控件:

- itemTipsWidget

首先在插件核心类中添加一个文本控件作为 tip 控件：

```c++
// homemonitorplugin.h
private:
    InformationWidget *m_pluginWidget;
    QLabel *m_tipsWidget; // new
```

在 init 函数中初始化：

```c++
// homemonitorplugin.cpp

void HomeMonitorPlugin::init(PluginProxyInterface *proxyInter)
{
    m_proxyInter = proxyInter;

    m_pluginWidget = new InformationWidget;
    m_tipsWidget = new QLabel; // new

    // 如果插件没有被禁用则在初始化插件时才添加主控件到面板上
    if (!pluginIsDisable()) {
        m_proxyInter->itemAdded(this, pluginName());
    }
}
```

下面在插件核心类中实现接口 itemTipsWidget：

```c++
// homemonitorplugin.h
public:
    QWidget *itemTipsWidget(const QString &itemKey) override;
```

```c++
// homemonitorplugin.cpp

QWidget *HomeMonitorPlugin::itemTipsWidget(const QString &itemKey)
{
    Q_UNUSED(itemKey);

    // 设置/刷新 tips 中的信息
    m_tipsWidget->setText(QString("Total: %1G\nAvailable: %2G")
                          .arg(qRound(m_pluginWidget->storageInfo()->bytesTotal() / qPow(1024, 3)))
                          .arg(qRound(m_pluginWidget->storageInfo()->bytesAvailable() / qPow(1024, 3))));

    return m_tipsWidget;
}
```

dde-dock 发现鼠标悬停在插件上时，会调用这个接口获取控件并显示出来，如下图所示：

![tips-widget](images/tips-widget.png)

#### 支持 applet

tip 控件在鼠标指针移开之后就会消失，如果插件需要长时间显示一个窗体(即使鼠标指针移开也会 保持显示状态)来实现一些提示或功能的话，就需要使用 applet 控件。applet 控件在单击后显示，在单击控件以外的其他地方后消失。
applet 控件其实跟 tip 控件一样，都是普通的 Widget，但是可以在 applet 控件中显示交互性的内容，比如按钮、输入框等。
由于篇幅限制，这里对于 applet 控件就不介绍添加交互性的功能了，它只用来显示一些文字，所以依然使用 label 控件。在插件核心类 homemonitorplugin 中添加一个文本控件作为 applet 控件:

```c++
// homemonitorplugin.h
private:
    InformationWidget *m_pluginWidget;
    QLabel *m_tipsWidget;
    QLabel *m_appletWidget; // new
```

在 init 函数中初始化：

```c++
// homemonitorplugin.cpp

void HomeMonitorPlugin::init(PluginProxyInterface *proxyInter)
{
    m_proxyInter = proxyInter;

    m_pluginWidget = new InformationWidget;
    m_tipsWidget = new QLabel;
    m_appletWidget = new QLabel; // new

    // 如果插件没有被禁用则在初始化插件时才添加主控件到面板上
    if (!pluginIsDisable()) {
        m_proxyInter->itemAdded(this, pluginName());
    }
}
```

接着实现 applet 相关的接口 itemPopupApplet：

```c++
// homemonitorplugin.h
public:
    QWidget *itemPopupApplet(const QString &itemKey) override;
```

```c++
// homemonitorplugin.cpp
QWidget *HomeMonitorPlugin::itemPopupApplet(const QString &itemKey)
{
    Q_UNUSED(itemKey);

    m_appletWidget->setText(QString("Total: %1G\nAvailable: %2G\nDevice: %3\nVolume: %4\nLabel: %5\nFormat: %6\nAccess: %7")
                            .arg(qRound(m_pluginWidget->storageInfo()->bytesTotal() / qPow(1024, 3)))
                            .arg(qRound(m_pluginWidget->storageInfo()->bytesAvailable() / qPow(1024, 3)))
                            .arg(QString(m_pluginWidget->storageInfo()->device()))
                            .arg(m_pluginWidget->storageInfo()->displayName())
                            .arg(m_pluginWidget->storageInfo()->name())
                            .arg(QString(m_pluginWidget->storageInfo()->fileSystemType()))
                            .arg(m_pluginWidget->storageInfo()->isReadOnly() ? "ReadOnly" : "ReadWrite")
                            );

    return m_appletWidget;
}
```

编译，安装，重启 dde-dock，点击主控件即可看到弹出的 applet 控件，如下图所示：

![applet-widget](images/applet-widget.png)

#### 支持右键菜单

增加右键菜单功能需要实现以下两个接口：itemContextMenu 和 invokedMenuItem，代码如下：

```c++
// homemonitorplugin.h
public:
    const QString itemContextMenu(const QString &itemKey) override;
    void invokedMenuItem(const QString &itemKey, const QString &menuId, const bool checked) override;
```

```c++
// homemonitorplugin.cpp
const QString HomeMonitorPlugin::itemContextMenu(const QString &itemKey)
{
    Q_UNUSED(itemKey);

    QList<QVariant> items;
    items.reserve(2);

    QMap<QString, QVariant> refresh;
    refresh["itemId"] = "refresh";
    refresh["itemText"] = "Refresh";
    refresh["isActive"] = true;
    items.push_back(refresh);

    QMap<QString, QVariant> open;
    open["itemId"] = "open";
    open["itemText"] = "Open Gparted";
    open["isActive"] = true;
    items.push_back(open);

    QMap<QString, QVariant> menu;
    menu["items"] = items;
    menu["checkableMenu"] = false;
    menu["singleCheck"] = false;

    // 返回 JSON 格式的菜单数据
    return QJsonDocument::fromVariant(menu).toJson();
}

void HomeMonitorPlugin::invokedMenuItem(const QString &itemKey, const QString &menuId, const bool checked)
{
    Q_UNUSED(itemKey);

    // 根据上面接口设置的 id 执行不同的操作
    if (menuId == "refresh") {
        m_pluginWidget->storageInfo()->refresh();
    } else if ("open") {
        QProcess::startDetached("gparted");
    }
}
```

编译，安装，重启 dde-dock 之后右键点击主控件即可看到弹出右键菜单，如下图所示：

![context-menu](images/context-menu.png)

至此，一个包含基本功能的插件就完成了。
