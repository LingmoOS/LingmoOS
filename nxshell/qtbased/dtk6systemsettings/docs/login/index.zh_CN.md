@page dtklogin dtklogin
@brief dtk登录管理模块

# dtklogin

## 项目介绍

dtklogin是对systemd-logind的DBus接口和dde-daemon提供的相关接口的封装。用于观察和管理用户 login 以及 seat 的状态。<br>
@ref group_dtklogin "接口文档"

## 项目结构

该模块一共提供四个类：

+ `DLoginManager` 管理器对象，可直接构造
+ `DLoginSeat` Seat对象，可通过DLoginManager获取
+ `DLoginSession` 会话对象，可通过DLoginManager获取
+ `DLoginUser` 用户对象，可通过DLoginManager获取

## 配置管理

登录管理的配置文件存在于下列文件中：

```shell
/etc/systemd/logind.conf
/etc/systemd/logind.conf.d/*.conf
/run/systemd/logind.conf.d/*.conf
/usr/lib/systemd/logind.conf.d/*.conf
```

## 说明

这些文件配置了 systemd 登录管理器的各种参数，有关语法的描述，请参见[systemd.syntax](https://www.freedesktop.org/software/systemd/man/systemd.syntax.html)。

## 配置目录和优先级

默认配置是在编译期间设置的，因此只有在需要改变这些默认配置时才需要配置。主配置文件/etc/systemd/logind.conf包含注释掉的条目，其显示的默认值作为管理员的指南。可以通过编辑此文件或创建插件来进行本地覆盖。建议使用插件进行本地配置，而不是修改主配置文件。<br>

除了主配置文件之外，还从/usr/lib/systemd/*.conf.d/、/usr/local/lib/systemd/*.conf.d/和/etc/systemd/*.conf.d/这些目录中读取插件. 这些插件具有更高的优先级并覆盖主配置文件。*.conf.d/子目录中的配置文件按其文件名字典顺序排序，而不管它们位于哪个子目录中。当多个文件指定相同的选项时，对于仅接受单个值的选项，文件中最后排序的条目优先；对于接受列表值的选项，条目在排序文件中出现时被收集。<br>

当包需要自定义配置时，可以在/usr/中安装插件。/etc/的文件是为本地管理员保留的，他们可以使用此逻辑覆盖供应商软件包安装的配置文件。必须使用插件来覆盖包插件，因为主配置文件的优先级较低。建议在这些子目录中的所有文件名前加上一个两位数字和一个破折号，以简化文件的排序。<br>

要禁用供应商提供的配置文件，推荐的方法是在/etc目录中放置一个/dev/null符号链接，其文件名与供应商配置文件相同。

## 使用实例

`main.cpp`

```cpp
#include <QCoreApplication>
#include "demo.h"
#include "signaldemo.h"
#include "propertydemo.h"
#include "powerdemo.h"

DLOGIN_USE_NAMESPACE

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QList<Demo *> demos = {new SignalDemo(&app), new PropertyDemo(&app), new PowerDemo(&app)};
    foreach (const auto demo, demos) {
        demo->run();
    }
    return app.exec();
}
```

`demo.h`

```cpp
#pragma once
#include <qobject.h>

class Demo : public QObject
{
public:
    Demo(QObject *parent = nullptr);
    virtual int run() = 0;
};
```

`demo.cpp`

```cpp
#include "demo.h"

Demo::Demo(QObject *parent)
    : QObject(parent)
{
}
```

`propertydemo.h`

```cpp

#include "demo.h"
#include "dloginmanager.h"
#include "dloginsession.h"
DLOGIN_USE_NAMESPACE
class PropertyDemo : public Demo
{
    Q_OBJECT
public:
    PropertyDemo(QObject *parent = nullptr);
    int run() override;

private:
    DLoginManager *m_manager;
    QSharedPointer<DLoginSession> m_currentSession;
};
```

`propertydemo.cpp`

```cpp

#include "propertydemo.h"
#include <qdebug.h>
#include <QMetaObject>
#include <QMetaProperty>
#include "dloginuser.h"
#include "dloginseat.h"

PropertyDemo::PropertyDemo(QObject *parent)
    : Demo(parent)
    , m_manager(new DLoginManager)
    , m_currentSession(m_manager->currentSession())
{
}

int PropertyDemo::run()
{
    const QMetaObject *mo = m_manager->metaObject();
    for (int i = 0; i < mo->propertyCount(); i++) {
        QMetaProperty mp = mo->property(i);
        qDebug() << "Property:" << mp.name() << ", type:" << mp.typeName() << ", readable:" << mp.isReadable()
                 << ", writable:" << mp.isWritable() << ", value:" << mp.read(m_manager);
    }
    qDebug() << m_manager->scheduledShutdown();
    auto user = m_manager->findUserById(1000);
    mo = user->metaObject();
    for (int i = 0; i < mo->propertyCount(); i++) {
        QMetaProperty mp = mo->property(i);
        qDebug() << "Property:" << mp.name() << ", type:" << mp.typeName() << ", readable:" << mp.isReadable()
                 << ", writable:" << mp.isWritable() << ", value:" << mp.read(user.data());
    }
    qDebug() << user->UID();
    auto session = m_manager->findSessionById(m_currentSession->id());
    mo = session->metaObject();
    for (int i = 0; i < mo->propertyCount(); i++) {
        QMetaProperty mp = mo->property(i);
        qDebug() << "Property:" << mp.name() << ", type:" << mp.typeName() << ", readable:" << mp.isReadable()
                 << ", writable:" << mp.isWritable() << ", value:" << mp.read(session.data());
    }
    qDebug() << session->id();
    auto seat = m_manager->currentSeat();
    mo = seat->metaObject();
    for (int i = 0; i < mo->propertyCount(); i++) {
        QMetaProperty mp = mo->property(i);
        qDebug() << "Property:" << mp.name() << ", type:" << mp.typeName() << ", readable:" << mp.isReadable()
                 << ", writable:" << mp.isWritable() << ", value:" << mp.read(seat.data());
    }
    qDebug() << seat->id();
    return 0;
}
```

既可使用元对象系统获取属性，也可以通过函数获取

`signaldemo.h`

```cpp
#pragma once
#include "demo.h"
#include "dloginmanager.h"

DLOGIN_USE_NAMESPACE
class SignalDemo : public Demo
{
    Q_OBJECT

public:
    SignalDemo(QObject *parent = nullptr);
    int run() override;

private:
    DLoginManager *m_manager;
    QSharedPointer<DLoginSession> m_currentSession;
};
```

`signaldemo.cpp`

```cpp
#include "signaldemo.h"
#include "dloginsession.h"
#include <qdebug.h>
#include <qfile.h>
#include <qdir.h>
#include <qtextstream.h>
SignalDemo::SignalDemo(QObject *parent)
    : Demo(parent)
    , m_manager(new DLoginManager)
    , m_currentSession(m_manager->currentSession())
{
}

int SignalDemo::run()
{
    connect(m_currentSession.data(), &DLoginSession::lockedChanged, this, [&](bool locked) {
        qDebug() << "Lock changed:" << locked;
    });
    connect(m_manager, &DLoginManager::userNew, this, [=](quint32 UID) { qDebug() << "User" << UID << "is added."; });
    connect(m_manager, &DLoginManager::userRemoved, this, [=](quint32 UID) { qDebug() << "User" << UID << "is removed."; });
    connect(m_manager, &DLoginManager::sessionNew, this, [=](const QString sessionId) {
        qDebug() << "Session" << sessionId << "is added.";
    });
    connect(m_manager, &DLoginManager::sessionRemoved, this, [=](const QString &sessionId) {
        qDebug() << "Session" << sessionId << "is removed.";
    });
    connect(m_currentSession.data(), &DLoginSession::autostartAdded, this, [=](const QString name) {
        qDebug() << "Autostart" << name << "is added.";
    });
    connect(m_currentSession.data(), &DLoginSession::autostartRemoved, this, [=](const QString &name) {
        qDebug() << "Autostart" << name << "is removed.";
    });
    connect(
        m_manager, &DLoginManager::prepareForSleep, this, [=](bool value) { qDebug() << "Prepare for sleep, value:" << value; });
    connect(m_manager, &DLoginManager::prepareForShutdown, this, [=](bool value) {
        qDebug() << "Prepare for shutdown, value:" << value;
        QFile file(QDir::currentPath() + "/shutdown.log");
        while (!file.open(QIODevice::ReadWrite | QIODevice::Append))
            ;
        QTextStream stream(&file);
        stream << QDateTime::currentDateTime().toString() << ": value = " << value;
        file.close();
    });

    return 0;
}
```

可直接连接对象的信号

`powerdemo.h`

```cpp
#pragma once
#include "demo.h"
#include "dloginmanager.h"
DLOGIN_USE_NAMESPACE

class PowerDemo : public Demo
{
    Q_OBJECT
public:
    PowerDemo(QObject *parent = nullptr);
    int run() override;

private:
    DLoginManager *m_manager;
};
```

`powerdemo.cpp`

```cpp
#include "powerdemo.h"
#include <qdebug.h>
#include <iostream>
#include <string>
#include "dlogintypes.h"

PowerDemo::PowerDemo(QObject *parent)
    : Demo(parent)
    , m_manager(new DLoginManager)
{
}

int PowerDemo::run()
{
    m_manager->inhibit(InhibitBehavior::Sleep, "demo", "block reboot", InhibitMode::Block);
    m_manager->inhibit(InhibitBehavior::Shutdown, "demo", "delay shutdown", InhibitMode::Delay);
    qDebug() << m_manager->listInhibitors();
    qDebug() << m_manager->canSuspend();
    qDebug() << m_manager->canPowerOff();
    auto time = QDateTime::currentDateTime();
    m_manager->scheduleShutdown(ShutdownType::Reboot, time.addSecs(1000));
    qDebug() << m_manager->scheduledShutdown();
    m_manager->cancelScheduledShutdown();
    qDebug() << m_manager->scheduledShutdown();
    std::string str;
    getline(std::cin, str);
    m_manager->powerOff(false);
    return 0;
}
```

调用相关方法可以执行系统相关操作，并触发信号

@defgroup dtklogin
@brief dtklogin模块是对于系统登陆相关的操作的封装，包括用户、会话、自启动、电源管理等
@details 示例文档:
@subpage dtklogin
@anchor group_dtklogin
