
# dde-launcher 开发文档

# 整体设计

由于 `dde-launcher` 需要在全屏模式和窗口模式来回切换，并且还注册了 DBus 服务。为了方便处理，使用 `LauncherSys` 作为中心控制类。

根据当前需要，`LauncherSys` 将选择性的切换显示全屏与窗口模式的实例。同时，DBus 服务也绑定在这个对象上，在调用时根据当前情况将对应的操作进行分发。

## 基本模块间关系

所有用来显示的模块（`FullScreenFrame`、`MiniFrame`、`WindowedFrame`）及与它们搭配的 `Delegate` 都只负责前端界面的处理，所有前端界面的数据都是从同一个 `AppsListModel` 中获取到的。在界面上进行的各种动画，也都是由各个 View 配合 Model 中保存的数据来完成的。

`AppsManager` 是一个对数据进行整理、封装，处理数据变动与更新的管理模块。其底层访问 `dde-session-initializater` 提供的 DBus 服务来获取应用列表及相关数据信息。对上则为 `AppListModel` 提供整合后的应用数据列表。

`AppItemDelegate` 与 `AppListDelegate` 分别用作全屏模式与窗口模式的绘图模块。它利用标准的 Qt Model-View 设计模式做到了模块间的分离。

## FullscreenFrame

在全屏模式下，有两种显示效果，即单 View 显示（如搜索界面或不分类模式）和多 View 显示（分类模式下）。

无论哪种模式下，所有的 View 都是被放置在一个纵向自适应增长的 VBoxLayout 上面，也是在这一层进行鼠标滚动的处理。即：__所有的 View 都是以 FitToSize 的方式进行展开的，滚动由它们的父控件处理__。这样可以把鼠标事件直接拿到最顶层处理，避免各个子控件在互相处理事件时带来混乱。

## WindowdFrame

在窗口模式下，数据来源及操作方式都和全屏模式相同，只需要替换掉显示的 View 和绘图的 Delegate 即可，参考 `LauncherSys` 中的相关代码。

## 输入法

由于特殊的需求，要求在按下按键时，立即跳转到搜索框中并开始搜索。为了保证输入法在按下第一个键之前就已经激活并能够正常捕获第一个字符，需要提前激活输入法。

为了实现这个功能，将窗口“伪装”成可输入区域，激活输入法并实现 `inputMethodEvent`、`inputMethodQuery` 等接口，实现与输入法的通讯，并接受输入法提交的结果转发到搜索栏中进行搜索。

# 模块设计

## Model

由于涉及到后端数据变动、应用分类及一些数据更新，在 `AppsListModel` 中又额外加了一层包装 `AppsManager`，单独用作对 DBus 数据进行过滤、分类等处理，然后将整理好的结构化的数据交给上层 `AppsListModel` 使用。

同时，为了减少 DBus 调用，一些 Cache 也在 `AppsManager` 中实现。

## View

目前有 `WindowedFrame` 和 `FullscreenFrame` 正在使用，它们都以 `AppsListModel` 作为数据源来显示应用列表。当以后有新设计需求时，可以添加新的 `View` 模块，实现对应的 Delegate 绘图即可接入。

## Delegate

根据 View 的特点和设计风格，从 `AppsListModel` 中获取数据进行绘图。

## Menu

`Menu` 模块是用于显示 Item 右键菜单的，无论在哪个 View 下，都使用这个 Menu 进行菜单显示，对应的操作由 View 及 `AppsManager` 处理。

## Utils

`CalculateUtils` 类负责进行基本的数据运算，例如不同模式下的图标大小、字体大小计算。

`Utils` 类是一些 qss 加载函数、图标查找函数等杂项的存放。

`RecentlyUsed` 类负责解析和处理最近使用的文件列表，它是读取 `~/.local/share/recently-used.xbel` 进行显示的，这主要是之前的一个定制功能，没有做的很仔细，由 `HISTORY_PANEL` 选项控制。

# 接口设计

## DBus 服务接口

Launcher 通过 DBus 服务提供了以下接口：

- Exit： 结束当前进程
- Hide： 隐藏当前 Launcher
- Show： 显示当前 Launcher
- ShowByMode： 以指定模式显示 Launcher，当前未实现此接口
- UninstallApp： 由于卸载 App 需要确认，调用此对话框即可到达确认界面
- Toggle： 切换 Launcher 的显示/隐藏模式
- IsVisible： 获取当前 Launcher 的显示状态



# 性能优化

## 修改之前的方案:

- 问题1:  开启了独立线程加载应用资源，但缺乏有效控制，没有实现等待资源加载完成后再显示界面．
- 问题2: 日历图标没有加入缓存，界面刷新时存在多线程竞争访问QIcon的情况，还会一直在主线程中查找，耗费系统资源．
- 问题3:  之前忽略了搜索栏和导航栏在未加入主界面布局前通过高度sizeHint().height() 获取不对的情况，导致部分应用资源没有加载到缓存，也就进一步加大了多线程访问QIcon对象的几率．
- 问题4:  除小窗口外，缓存了5个档级下所有应用资源的图标，没有动态释放缓存，内存占用较大．

## 启动器预加载图标的前提：

- 提前知道搜索栏和导航栏的高度信息．
- 目前全屏自由以及全屏分类模式都需要知道搜索栏的高度．

- 因为条件1无法满足，因此不满足预加载图标的基本条件．

## 现行的优化方案:

为简化描述，使用以下简称替代下:

- 小窗口: small
- 全屏自由模式: full free
- 全屏分类模式: category
- 切换: -->
- 双向切换: <-->

1. small --> full free / category, 删除 small 缓存.
2. full free <--> category时，不缓存小窗口，只缓存全屏下另一种模式(如果当前是full free,那就缓存当前ratio下的category资源，其他情况同理)，直接在加入布局后，使用搜索栏和导航栏的sizeHint大小计算当前界面模式下每个图标大小，通过dbus监听模式变化，加载FullscreenFrame对象，在构造中触发独立线程加载应用资源，加载完毕后再显示主界面．
3. full free / category --> small, 不删除category或者full free对应的缓存，因为全屏下的缓存存在5个档级的计算，此外当界面显示后，如果想在独立线程中去加载应用资源，会出现主线程和独立线程竞相加载，造成多线程同时访问QIcon对象的事故，容易导致内存崩溃．
4. 日历加入缓存，每分钟获取下当前的天数，超过今天，获取新的日历图标．
4.  category时，如果修改ratio，则信号通知独立线程，加载另一种模式新的ratio下的资源．

- 综上, 此次性能优化方案是：不做资源预加载，当独立线程中应用资源加载完毕后，再启动器显示主界面．
- 全屏下，清除小窗口的缓存．
