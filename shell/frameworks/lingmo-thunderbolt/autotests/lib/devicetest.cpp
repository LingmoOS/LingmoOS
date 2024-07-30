/*
 * SPDX-FileCopyrightText: 2018-2019 Daniel Vrátil <dvratil@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include <QObject>
#include <QSignalSpy>
#include <QTest>

#include "fakedevice.h"
#include "fakemanager.h"
#include "fakeserver.h"

#include "device.h"
#include "manager.h"

#include <memory>

class DeviceTest : public QObject
{
    Q_OBJECT
public:
    DeviceTest()
        : QObject()
    {
        FakeServer::enableFakeEnv();
        qRegisterMetaType<QSharedPointer<Bolt::Device>>();
    }

private Q_SLOTS:
    void testAuthorize()
    {
        std::unique_ptr<FakeServer> fakeServer;
        try {
            fakeServer = std::make_unique<FakeServer>();
        } catch (const FakeServerException &e) {
            qWarning("Fake server exception: %s", e.what());
            QFAIL("Caught server exception");
        }

        auto fakeManager = fakeServer->manager();
        FakeDevice *fakeDevice = nullptr;
        try {
            fakeDevice = fakeManager->addDevice(std::make_unique<FakeDevice>(QStringLiteral("Device1")));
        } catch (const FakeDeviceException &e) {
            qWarning("Fake device exception: %s", e.what());
            QFAIL("Caught device exception");
        }
        fakeDevice->setAuthFlags(QStringLiteral("none"));

        Bolt::Manager manager;
        QVERIFY(manager.isAvailable());

        auto device = manager.device(fakeDevice->uid());
        QVERIFY(device);
        QCOMPARE(device->authFlags(), Bolt::Auth::None);
        device->authorize(Bolt::Auth::NoKey | Bolt::Auth::Boot);

        QTRY_COMPARE(fakeDevice->authFlags(), QStringLiteral("nokey | boot"));
    }
};

#include "devicetest.moc"

QTEST_GUILESS_MAIN(DeviceTest)
