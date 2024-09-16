@page dtkpower dtkpower
@brief dtk电源管理模块

# dtkpower

## 项目介绍

dtkpower是对于deepin/UOS系统上的upowerdbus接口和dde-daemon接口的封装，其目的是在于方便第三方开发者轻松且快速的调用接口进行开发。<br>
@ref group_power "接口文档"


## 项目结构

对外暴露出 `dkbdbacklight.h` `dpowerdevice.h` `dpowermanager.h`如果需要对键盘背光和电源信息获取，需要使用dpowermanager进行管理。见下方示例

# 如何使用项目

如果要使用此项目，可以阅读相关文档再参照以下示例进行调用

## dkbdbacklight使用示例

```cpp
#include "dpowermanager.h"//为了创建manager对象需要调用此头文件
#include "dkbdbacklight.h" //为了管理键盘背光需要调用此头文件
#include <QCoreApplication>
#include <qdebug.h>

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    Dtk::Power::DPowerManager manager; //创建manager对象
    auto kb = manager.kbdBacklight();//使用manager对象创建键盘对象并获取
    qDebug() << kb->brightness();//调用对应方法，获取当前键盘灯亮度
    qDebug() << "*************************************************";
    kb->setBrightness(1);//调整键盘灯亮度
    QObject::connect(kb.data(), &Dtk::Power::DKbdBacklight::brightnessChanged, &app, [=](const qint32 value) {
        qDebug() << "brightness:" << value;
    });
    QObject::connect(
        kb.data(),
        &Dtk::Power::DKbdBacklight::brightnessChangedWithSource,
        &app,
        [=](const qint32 value, const KbdSource &source) { qDebug() << "brightness:" << value << static_cast<int>(source); });//连接信号并且用lambda表达式捕获信号值强制转换为int类型输出
    return app.exec();
}
```

## dpowerdevice使用示例

和上方的使用方法类似

```cpp
#include "dpowermanager.h"
#include "dpowerdevice.h"
#include <QCoreApplication>
#include <qdebug.h>

int main(int argc, char **argv)
{
    QCoreApplication a(argc, argv);
    Dtk::Power::DPowerManager manager; //创建manager对象
    auto device = manager.displayDevice(); //由manager对象创建device对象
    qDebug() << device->energyFullDesign();
        QObject::connect(device.data(), &Dtk::Power::DPowerDevice::updateTimeChanged, &app, [=](const QDateTime &value) {
        qDebug() << "updateTime:" << value;
    });//连接信号 并且将信号传出的值用lambda表达式捕获并且输出
    QObject::connect(device.data(), &Dtk::Power::DPowerDevice::timeToEmptyChanged, &app, [=](const qint64 value) {
        qDebug() << "Time:" << value;
    });
    QObject::connect(device.data(), &Dtk::Power::DPowerDevice::percentageChanged, &app, [=](const double value) {
        qDebug() << "percentage:" << value;
    });
    QObject::connect(
        &manager, &Dtk::Power::DPowerManager::lidIsClosedChanged, &app, [=](const bool value) { qDebug() << "Lid:" << value; }); //这个是powermanager的信号
    QObject::connect(
        &manager, &Dtk::Power::DPowerManager::deviceAdded, &app, [=](const QString &name) { qDebug() << "name:" << name; });
    QObject::connect(
        &manager, &Dtk::Power::DPowerManager::deviceRemoved, &app, [=](const QString &name) { qDebug() << "nameRM:" << name; });
    return app.exec();
}
```

但是和上面的不同的是，manager提供了几种不同的方法创建device对象，分别为 `displayDevice` `findDeviceByName` 如果需要对比其用法，详见doxygen文档

## dpowersettings使用指南

dpowersetting是对于电脑的电源进行相关设置，这些设置是（目前）依赖于dde-daemon实现的，所以在不存在dde-daemon的电脑上是无法进行调用。这个的调用方法就很简单，只需要直接创建dpowersettings对象即可。

需要注意的是，在设置一些电源选项（如混合睡眠，休眠)之前最好通过dtklogin对应方法确定其是否被支持，否则可能会出现意料之外的错误（如电脑黑屏无法被唤醒的情况)

@defgroup dtkpower
@brief 电源管理模块
@details 示例文档:
@subpage dtkpower
@anchor group_dtkpower
