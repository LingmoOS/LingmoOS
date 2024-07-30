/*
 * SPDX-FileCopyrightText: 2019 Manuel Weichselbaumer <mincequi@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "leserver.h"

#include <QCoreApplication>
#include <QDebug>

#include "adapter.h"
#include "device.h"
#include "gattapplication.h"
#include "gattcharacteristic.h"
#include "gattdescriptor.h"
#include "gattmanager.h"
#include "gattservice.h"
#include "initmanagerjob.h"
#include "leadvertisement.h"
#include "leadvertisingmanager.h"
#include "manager.h"
#include "pendingcall.h"

using namespace BluezQt;

LeServer::LeServer(Manager *manager, QObject *parent)
    : QObject(parent)
    , m_manager(manager)
{
    auto advertisement = new LEAdvertisement({QStringLiteral("ad100000-d901-11e8-9f8b-f2801f1b9fd1")}, this);
    auto call = m_manager->usableAdapter()->leAdvertisingManager()->registerAdvertisement(advertisement);
    connect(call, &PendingCall::finished, this, [](BluezQt::PendingCall *call) {
        if (call->error()) {
            qWarning() << "Register advertisement error: " << call->errorText();
            return;
        }
    });

    auto application = new GattApplication(QStringLiteral("/org/kde/bluezqt"), this);
    auto service = new GattService(QStringLiteral("ad100000-d901-11e8-9f8b-f2801f1b9fd1"), true, application);

    GattCharacteristic *characteristic = new GattCharacteristic(QStringLiteral("ad10e100-d901-11e8-9f8b-f2801f1b9fd1"), service);
    GattDescriptor::createUserDescription(QLatin1String("MyCharacteristic"), characteristic);

    m_notifyingCharacteristic =
        new GattCharacteristic(QStringLiteral("ad10e100-d902-11e8-9f8b-f2801f1b9fd1"), {QStringLiteral("read"), QStringLiteral("notify")}, service);

    auto call2 = m_manager->usableAdapter()->gattManager()->registerApplication(application);

    connect(call2, &PendingCall::finished, this, [](BluezQt::PendingCall *call) {
        if (call->error()) {
            qWarning() << "Register application error: " << call->errorText();
            return;
        }
    });

    m_characteristicWriteTimer.setInterval(std::chrono::seconds(1));
    m_characteristicWriteTimer.setSingleShot(false);
    m_characteristicWriteTimer.start();

    static int charValue = 1;
    QObject::connect(&m_characteristicWriteTimer, &QTimer::timeout, [this]() {
        m_notifyingCharacteristic->writeValue(QByteArray::number(charValue++));
    });
}

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    qDebug() << "Advertising LE services. Ctrl + C to cancel...";

    Manager *manager = new Manager();
    InitManagerJob *initJob = manager->init();
    initJob->exec();
    if (initJob->error()) {
        qWarning() << "Error initializing manager:" << initJob->errorText();
        return 1;
    }

    if (!manager->usableAdapter()) {
        qWarning() << "No usable adapter";
        return 2;
    }
    if (!manager->usableAdapter()->leAdvertisingManager()) {
        qWarning() << "No LE advertising manager";
        return 2;
    }
    if (!manager->usableAdapter()->gattManager()) {
        qWarning() << "No GATT manager";
        return 2;
    }

    new LeServer(manager);

    return app.exec();
}

#include "moc_leserver.cpp"
