@page dtkaccounts dtkaccounts
@brief dtk用户管理模块

# dtkaccounts

## 项目介绍

dtkaccounts是对于deepin/UOS系统上的org.freedesktop.Accounts的dbus接口和com.deepin.daemon.Accounts的dbus接口的封装，同时使用Qt以及Linux原生接口实现了一部分功能，其目的是在于方便第三方开发者轻松且快速的调用接口进行开发。<br>
@ref group_accounts "接口文档"

## 项目结构

对外暴露出 `daccountsmanager.h` `daccountsuser.h` `daccountstypes.h`这三个类，用户和账户信息的管理通过构造其对象来进行操作。

## 如何使用项目

如果要使用此项目，可以阅读相关文档再参照以下示例进行调用

## 使用示例

可参考以下demo：

`main.cpp`

```cpp
#include "demo.h"
#include <QCoreApplication>

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    Demo d;
    d.run();
    return app.exec();
}
```

`demo.h`

```cpp
#pragma once

#include "daccountsmanager.h"
#include "daccountsuser.h"
#include "daccountstypes.h"
#include <QObject>

DACCOUNTS_USE_NAMESPACE

class Demo : public QObject
{
    Q_OBJECT
public:
    Demo();
    void run();

private:
    DAccountsManager manager;
    QSharedPointer<DAccountsUser> user{nullptr};
};
```

`demo.cpp`

```cpp
#include "demo.h"
#include <QDebug>

Demo::Demo()
{
    user = manager.findUserByName("test"); //创建DaccountsUser对象的方法之一，剩下还有createUser和findUserById

    //以下信号按需连接
    connect(&manager, &DAccountsManager::UserAdded, this, [](const quint64 uid) { qDebug() << "new user add:" << uid; });
    connect(&manager, &DAccountsManager::UserDeleted, this, [](const quint64 uid) { qDebug() << "delete user:" << uid; });
    connect(user.data(), &DAccountsUser::automaticLoginChanged, this, [](const bool) { qDebug() << "automaticLoginChanged"; });
    connect(user.data(), &DAccountsUser::noPasswdLoginChanged, this, [](const bool) { qDebug() << "noPasswdLoginChanged"; });
    connect(user.data(), &DAccountsUser::groupsChanged, this, [](const QStringList &) { qDebug() << "groupsChanged"; });
    connect(user.data(), &DAccountsUser::iconFileListChanged, this, [](const QList<QByteArray> &) {
        qDebug() << "iconFileListChanged";
    });
    connect(user.data(), &DAccountsUser::iconFileChanged, this, [](const QUrl &) { qDebug() << "iconFileChanged"; });
    connect(
        user.data(), &DAccountsUser::layoutListChanged, this, [](const QList<QByteArray> &) { qDebug() << "layoutListChanged"; });
    connect(user.data(), &DAccountsUser::layoutChanged, this, [](const QByteArray &) { qDebug() << "layoutChanged"; });
    connect(user.data(), &DAccountsUser::maxPasswordAgeChanged, this, [](const qint32) { qDebug() << "maxPasswordAgeChanged"; });
    connect(user.data(), &DAccountsUser::passwordHintChanged, this, [](const QString &) { qDebug() << "passwordHintChanged"; });
    connect(user.data(), &DAccountsUser::localeChanged, this, [](const QByteArray &) { qDebug() << "localeChanged"; });
    connect(user.data(), &DAccountsUser::lockedChanged, this, [](const bool) { qDebug() << "lockedChanged"; });
}

void Demo::run()
{
    manager.isUsernameValid("qwer");
    auto newuser = manager.createUser("qwer", "testqwer", AccountTypes::Default);

    manager.deleteUser("qwer", true);
    qDebug() << manager.groups();

    qDebug() << user->UID();
    qDebug() << user->UUID();
    user->setIconFile(QUrl("file:///var/lib/AccountsService/icons/13.png")); //注意QUrl其中参数格式要统一
    user->addGroup("libvirt");
    user->deleteGroup("libvirt");
    user->deleteIconFile(QUrl("file:///var/lib/AccountsService/icons/local/qwer-dfgdsd31dfs"));
    user->setLayout("cn;");
    user->setLayoutList({"cn;","jp;","en;"}); //对布局设置时注意格式
}
```

@defgroup dtkaccounts
@brief dtkaccounts是对于deepin/UOS系统上用户管理模块的封装<br>
@details 示例文档:
@subpage dtkaccounts
@anchor group_dtkaccounts
