@~chinese

\mainpage

## 项目介绍

dtknotifications是对于deepin/UOS系统上的org.freedesktop.Notifications的dbus接口和com.deepin.dde.Notification的dbus接口的封装，同时使用Qt以及Linux原生接口实现了一部分功能，其目的是在于方便第三方开发者轻松且快速的调用接口进行开发。

## 项目结构

对外暴露出 `dnotificationmanager.h` `dabstractnotificationmodeconfig.h` `dnotificationdndmodeconfig.h` `dnotificationappconfig.h` `dnotificationtypes.h`这五个类，系统通知和消息的管理通过构造其对象来进行操作。

## 如何使用项目

如果要使用此项目，可以阅读相关文档再参照以下示例进行调用

## 使用示例

可参考以下demo：

`main.cpp`

```cpp
int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    DNotificationManager manager;
    QObject::connect(&manager, &DNotificationManager::actionInvoked, []{ qDebug() << "actionInvoked"; });
    QObject::connect(&manager, &DNotificationManager::notificationClosed, []{ qDebug() << "notificationClosed"; });
    QObject::connect(&manager, &DNotificationManager::recordAdded, []{ qDebug() << "recordAdded"; });
    QObject::connect(&manager, &DNotificationManager::appNotificationConfigChanged, []{ qDebug() << "appInfoChanged"; });
    QObject::connect(&manager, &DNotificationManager::dndModeNotificationConfigChanged, []{ qDebug() << "systemInfoChanged"; });
    QObject::connect(&manager, &DNotificationManager::appAdded, []{ qDebug() << "appAdded"; });
    QObject::connect(&manager, &DNotificationManager::appRemoved, []{ qDebug() << "appRemoved"; });

    qDebug() << "*******************recordCount******************************";
    qDebug() << "recordCount:" << manager.recordCount().value();

    DNotificationData notificationData;
    qDebug() << "*******************allRecords******************************";
    auto records = manager.allRecords();
    bool first = true;
    QString offsetId;
    for (auto record : records.value()) {
        if (first) {
            first = false;
        }
        if (record.appName == "deepin-screen-recorder") {
            offsetId = record.id;
            notificationData = record;
        }
        qDebug() << record;
    }

    qDebug() << "******************getRecordsFromId*******************************";
    auto offsetRecords = manager.getRecordsFromId(2, offsetId);
    for (auto record : offsetRecords.value()) {
        qDebug() << record;
    }

    qDebug() << "*******************getRecordById******************************";
    auto record = manager.getRecordById(offsetId);
    qDebug() << offsetId << ":" << record.value();

    qDebug() << "*******************serverInformation******************************";
    qDebug() << manager.serverInformation().value();

    qDebug() << "*******************capbilities******************************";
    qDebug() << manager.capbilities().value();

    qDebug() << "*******************appList******************************";
    qDebug() << manager.appList().value();

    qDebug() << "*******************appInfo******************************";
    DNotificationAppConfigPtr appNotificationConfigPtr = manager.notificationAppConfig(notificationData.id);
    qDebug() << appNotificationConfigPtr->appName();

    qDebug() << "*******************setAppInfo******************************";
    QString appName("flameshot");
    appNotificationConfigPtr->setSoundEnabled(false);
    qDebug() << "soundEnabled:" << appNotificationConfigPtr->soundEnabled();

    qDebug() << "*******************systemInformation******************************";
    DNotificationDNDModeConfigPtr dndModeNotificatinConfigPtr = manager.notificationDNDModeConfig();
    qDebug() << "dndModeEnabled:" << dndModeNotificatinConfigPtr->DNDModeInLockScreenEnabled();

    qDebug() << "*******************setSystemInfo******************************";
    QString sysName("22:00");
    dndModeNotificatinConfigPtr->setStartTime(sysName);
    qDebug() << dndModeNotificatinConfigPtr->startTime();

    qDebug() << "*******************notify******************************";
    notificationData.appName = "new-application";
    notificationData.id = "2022";

    qDebug() << manager.notify(notificationData.appName, notificationData.replacesId, notificationData.appIcon, notificationData.summary).value();

    qDebug() << "*******************closeNotification******************************";
    QThread::msleep(2000);
    manager.closeNotification(notificationData.id.toUInt());

    manager.showNotificationCenter();

    qDebug() << "*******************hide******************************";
    QThread::msleep(1000);
    manager.hide();

    return app.exec();
}

```

