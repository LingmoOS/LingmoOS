### 简介
开始菜单(lingmo-menu)是LINGMO桌面环境的一部分，是一个简易的应用程序启动器，通常与LINGMO一起发布。
> Application launcher for lingmo desktop environment
lingmo-menu is a convenient application launcher which includes
normal and full-screen modes, it also has a plugin page which can be used
to load extra plugins like favourite apps page, recent files page, etc.

* 基本功能
1. 显示应用程序图标列表
2. 左键点击应用图标打开应用程序
3. 右键点击应用图标显示右键菜单

* 额外功能 (由开始菜单扩展插件实现)
1. 应用收藏夹
2. 最近文件列表
3. AI助手

### 扩展插件 (基于版本：upstream/4.10.1.0)
可扩展性是开始菜单的设计目标之一，目的是让用户可以更方便快捷的使用一些额外功能。
当然用户也可以自行编写自己的扩展插件，以下是关于扩展插件的简单介绍。

* 插件入口

扩展插件在开始菜单中被抽象为MenuExtensionPlugin类，该接口是开始菜单插件的主入口。基于该接口可以开发多种插件。
```c++
// 头文件路径为：src/extension/menu-extension-plugin.h
class Q_DECL_EXPORT MenuExtensionPlugin : public QObject
{
    Q_OBJECT
public:
    explicit MenuExtensionPlugin(QObject *parent = nullptr);
    ~MenuExtensionPlugin() override;

    /**
     * 插件的唯一id，会被用于区分插件
     * @return 唯一id
     */
    virtual QString id() = 0;

    /**
     * 创建一个Widget扩展
     * @return 返回nullptr代表不生产此插件
     */
    virtual WidgetExtension *createWidgetExtension() = 0;

    /**
     * 创建上下文菜单扩展
     * @return 返回nullptr代表不生产此插件
     */
    virtual ContextMenuExtension *createContextMenuExtension() = 0;
};
```

目前4.10.1.0版本的开始菜单提供了以下扩展点用于支持扩展开发：
1. 组件扩展(WidgetExtension)

该类型的插件需要提供一个ui界面显示在开始菜单的插件页。
```c++
// 头文件路径：src/extension/widget-extension.h
class WidgetExtension : public QObject
{
    Q_OBJECT
public:
    explicit WidgetExtension(QObject *parent = nullptr);
    virtual int index() const;
    virtual MetadataMap metadata() const = 0;

    // 兼容老版本
    virtual QVariantMap data();
    virtual void receive(const QVariantMap &data);

Q_SIGNALS:
    void dataUpdated();
};
```

2. 右键菜单扩展(ContextMenuExtension)

该类型的插件会向开始菜单的右键菜单中添加一些选择项，用于执行特定的操作。
```c++
// 头文件路径：src/extension/context-menu-extension.h
class ContextMenuExtension
{
public:
    virtual ~ContextMenuExtension() = default;
    /**
     * 控制菜单项显示在哪个位置
     * 对于第三方项目们应该选择-1,或者大于1000的值
     * @return -1：表示随机放在最后
     */
    virtual int index() const;

    /**
     * 根据data生成action,或者子菜单
     *
     * @param data app信息
     * @param parent action最终显示的QMenu
     * @param location 请求菜单的位置
     * @param locationId 位置的描述信息，可选的值有：all,category,letterSort和favorite等插件的id
     * @return
     */
    virtual QList<QAction*> actions(const DataEntity &data, QMenu *parent, const MenuInfo::Location &location, const QString &locationId) = 0;
};
```

* 插件开发实践

推荐使用cmake作为构建工具
> 建议参考此项目：https://gitee.com/lingmo/lingmo-menu-extensions.git

开始菜单提供cmake文件和pc(pkgconfig)文件用于导入开发文件。
```cmake
# 下面是基于cmake的大致步骤
find_package(lingmo-menu REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE
        lingmo-menu
)

# 下面是基于pkgconfig的大致步骤
find_package(PkgConfig REQUIRED)
pkg_check_modules(lingmo-menu REQUIRED lingmo-menu)
include_directories(lingmo-menu_INCLUDE_DIRS)
link_directories(lingmo-menu_LIBRARY_DIRS)
link_directories(lingmo-menu_LIBRARIES)
target_link_libraries(${PROJECT_NAME} PRIVATE lingmo-menu_LIBRARIES)
```

* 插件安装路径
```cmake
install(TARGETS ${EXTENSION_NAME} LIBRARY DESTINATION "/usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}/lingmo-menu/extensions")
```
