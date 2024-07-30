/*
 * SPDX-FileCopyrightText: 2018-2019 Daniel Vrátil <dvratil@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include <QDebug>
#include <QObject>
#include <QSignalSpy>
#include <QTest>

#include "fakedevice.h"
#include "fakemanager.h"
#include "fakeserver.h"

#include "device.h"
#include "manager.h"

#include <memory>

Q_DECLARE_METATYPE(QSharedPointer<Bolt::Device>)

class ManagerTest : public QObject
{
    Q_OBJECT
public:
    ManagerTest()
        : QObject()
    {
        FakeServer::enableFakeEnv();
        qRegisterMetaType<QSharedPointer<Bolt::Device>>();
    }

private Q_SLOTS:
    void testDeviceAddedRemoved()
    {
        std::unique_ptr<FakeServer> server;
        try {
            server = std::make_unique<FakeServer>();
        } catch (const FakeServerException &e) {
            qWarning("Fake server exception: %s", e.what());
            QFAIL("Exception server caught");
        }

        auto fakeManager = server->manager();

        Bolt::Manager manager;
        QVERIFY(manager.isAvailable());

        QSignalSpy addSpy(&manager, &Bolt::Manager::deviceAdded);
        QVERIFY(addSpy.isValid());

        FakeDevice *fakeDevice = nullptr;
        try {
            fakeDevice = fakeManager->addDevice(std::make_unique<FakeDevice>(QStringLiteral("device1")));
        } catch (const FakeDeviceException &e) {
            qWarning("Fake device exception: %s", e.what());
            QFAIL("Caught device exception");
        }
        QTRY_COMPARE(addSpy.size(), 1);
        auto device = addSpy.first().first().value<QSharedPointer<Bolt::Device>>();
        QCOMPARE(device->uid(), fakeDevice->uid());

        QSignalSpy removeSpy(&manager, &Bolt::Manager::deviceRemoved);
        QVERIFY(removeSpy.isValid());
        fakeManager->removeDevice(fakeDevice->uid());
        QTRY_COMPARE(removeSpy.size(), 1);
        QCOMPARE(removeSpy.first().first().value<QSharedPointer<Bolt::Device>>(), device);
    }
};

QTEST_GUILESS_MAIN(ManagerTest)

#include "managertest.moc"
