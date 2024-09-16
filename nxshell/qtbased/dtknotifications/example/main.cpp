// SPDX-FileCopyrightText: 2022 Uniontech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "dnotificationmanager.h"
#include "dnotificationtypes.h"

#include <QCoreApplication>
#include <QThread>
#include <QDebug>
#include <qeventloop.h>

DNOTIFICATIONS_USE_NAMESPACE

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

    qDebug() << "*******************showNotificationCenter******************************";
    manager.showNotificationCenter();

    qDebug() << "*******************hideNotificationCenter******************************";
    QThread::msleep(1000);
    manager.hideNotificationCenter();

    return app.exec();
}
