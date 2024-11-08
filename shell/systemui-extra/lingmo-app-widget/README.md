## lingmo-app-widget

lingmo-app-widget是LINGMO桌面环境的小插件框架，
插件用户可以使用小插件框架实现桌面环境小插件UI；

### 依赖

------

### 编译依赖

- debhelper-compat
- qt5-qmake
- qtbase5-dev
- qtbase5-dev-tools
- qtdeclarative5-dev
- qtquickcontrols2-5-dev
- libglib2.0-dev
- libgsettings-qt-dev
- liblingmo-log4qt-dev

### 编译

------

```shell
$ cd lingmo-app-widget
$ mkdir build
$ cd build
$ qmake ..
$ make
```

### 安装

------

```shell
$ sudo make install
```


### 框架组成
  - **InProgress**
    - 小插件管理服务
    - 小插件使用端
    - 小插件提供端
  - **TROUBLE**
    - 无
### SDK组成
  - **InProgress**
    - liblingmo-appwidget-manager
    - liblingmo-appwidget-plugin
    - liblingmo-appwidget-provider
    - AppWidget.qml
  - **TROUBLE**
    - 无

### 模块介绍
#### 框架
  - **管理服务模块**
     小插件框架中的后台管理服务，主要提供提供端与使用端
     的通信能力。
  - **使用端**
     用于显示小插件的应用组件，使用端可以不唯一，小插件使
     用端可以通过小插件框架提供的sdk获取到对应的小插件信息
     以及ui文件。
  - **提供端**
     应用组件作为提供端为小插件服务提供小插件ui界面，小插件
     的配置信息，小插件的更新数据服务。
  - **lingmo-appwidget-test**
     小插件框架应用的demo

##### SDK
  - **liblingmo-appwidget-manager**
     小插件管理服务接口SDK，用户可以通过该接口获取到小插件
     的信息，提供端可以通过管理服务接口推送更新数据。
  - **liblingmo-appwidget-plugin**
     封装为qml可以使用的内部插件库，由Appwidget.qml控件导入
     使用，是qml与小插件使用端的通信类，封装为点击事件消息、
     自定义事件消息、使用端注册dbus的接口以及数据更新的函数。
  - **liblingmo-appwidget-provider**
     小插件提供端SDK，提供端可以通过继承该类实现提供端的服务。
     接收由管理服务传来的消息。
  - **AppWidget.qml**
     qml控件的封装，用于小插件实例使用，负责连接qml与内部插件
     库函数，作为中间控件实现小插件实例与使用端的通信。
### 接口使用
#### 使用端接口
  - **获取小插件列表**
     QStringList getAppWidgetList();
     功能说明：获取可加载的小插件列表（列表形式如：("clock", "search")，列表内容为小插件名字）。
  - **获取小插件ui文件路径**
     QString getAppWidgetUiFile(const QString &appwidgetname);
     功能说明：通过小插件名字获取小插件qml ui布局文件（ui路径如："/usr/share/appwidget/qml/search.qml"）
  - **获取小插件的config属性**
     QString getAppWidgetConfig(const QString &appwidgetname,const AppWidgetConfig& config)
     功能说明：通过小插件名称，以及小插件的属性config获取小插件的配置信息
  - **获取小插件的所有属性**
     QMap<QString,QVariant> getAppWidgetConfigs(const QString &appwidgetname);
     功能说明：用于一次性获取小插件的所有配置信息
  - **更新使用端状态**
     void updateUserStatus(const QString &username, const UserStatus& userstatus);
     功能说明：用于使用端通知小插件使用端状态
  - **使用端注册小插件**
     register(QString appname, QString usrname);
     功能说明：用户使用端qml界面注册小插件，在使用端加载小插件时调用
#### 提供端接口
   创建提供端类，该类需要继承KAppWidgetProvider类
  - **小插件自定义事件**
     void appWidgetRecevie(const QString &eventname, const QString &widgetname, const QDBusVariant &value);
     功能说明：用于响应小插件qml中发送的自定义事件
  -  **小插件更新函数**
     void appWidgetUpdate();
     功能说明：用于响应由管理服务发送来的更新通知
  -  **小插件禁用函数**
     void appWidgetDisable();
     功能说明：用于响应由管理服务发送来的小插件禁用通知（当该提供端的所有小插件都被移除后发送）
  -  **小插件删除函数**
     void appWidgetDeleted();
     功能说明：用于响应由管理服务发送来的使用端小插件删除通知
  -  **小插件启用函数**
     void appWidgetEnable();
     功能说明：用于响应由管理服务发送来的小插件通知（当该提供端的小插件第一次被添加时发送）
  -  **小插件点击函数**
     void appWidgetClicked(const QString &widgetname, const QDBusVariant &value);
     功能说明：用于响应由管理服务发送来的小插件点击事件通知
