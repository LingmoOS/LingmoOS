@page dtkrfmanager dtkrfmanager
@brief dtk无线设备管理模块

# dtkrfmanager

## 项目介绍

`dtkrfmanager` 是对于 `deepin/UOS` 系统上的 rfkill 的封装，其目的是在于方便第三方开发者轻松且快速的调用接口管理无线设备。<br>
@ref group_rfmanager "接口文档"

## 项目结构

对外暴露出 `drfmanager.h` `dtkrfmanager_global.h` 这两个文件。

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

#include "drfmanager.h"
#include <QObject>

DACCOUNTS_USE_NAMESPACE

class Demo : public QObject
{
    Q_OBJECT
public:
    Demo();
    void run();

private:
    DRfmanager *m_rfMgr = nullptr;
};
```

`demo.cpp`

```cpp
#include "demo.h"
#include <QDebug>

Demo::Demo()
{
    m_rfMgr = new DRfmanager;

    //以下信号按需连接
    m_rfMgr = new DRfmanager;
    connect(m_rfMgr, &DRfmanager::bluetoothBlockedChanged, [](bool bluetoothBlocked){
        qDebug() << "bluetoothBlockedChanged" << bluetoothBlocked;
    });
    connect(m_rfMgr, &DRfmanager::wifiBlockedChanged, [](bool wifiBlocked){
        qDebug() << "wifiBlockedChanged" << wifiBlocked;
    });
    connect(m_rfMgr, &DRfmanager::countChanged, [](int count){
        qDebug() << "countChanged" << count;
    });
    connect(m_rfMgr, &DRfmanager::allBlockedChanged, [](bool blockAll){
        qDebug() << "allBlockedChanged" << blockAll;
    });
}

void Demo::run()
{
    m_rfMgr->count();
    m_rfMgr->blockWifi(true);
    m_rfMgr->isWifiBlocked();

    m_rfMgr->blockBluetooth(true);
    m_rfMgr->isBluetoothBlocked();

    m_rfMgr->blockAll(true);
    m_rfMgr->isblockAll();

    m_rfMgr->blockAll(true);
    m_rfMgr->isblockAll();
}
```
@defgroup dtkrfmanager
@brief dtk无线设备管理模块
@details 示例文档:
@subpage dtkrfmanager
@anchor group_dtkrfmanager
