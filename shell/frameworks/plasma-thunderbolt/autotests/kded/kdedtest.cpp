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

#include "kded_bolt.h"

#include <memory>

class TestableKDEDBolt : public KDEDBolt
{
    Q_OBJECT
public:
    using KDEDBolt::KDEDBolt;

Q_SIGNALS:
    void deviceNotify(const QList<QSharedPointer<Bolt::Device>> &device);

protected:
    void notify() override
    {
        Q_EMIT deviceNotify(sortDevices(mPendingDevices));
    }
};

class KDEDTest : public QObject
{
    Q_OBJECT
public:
    KDEDTest()
        : QObject()
    {
        FakeServer::enableFakeEnv();
        qRegisterMetaType<QSharedPointer<Bolt::Device>>();
    }

private Q_SLOTS:
    void testShouldNotify()
    {
        std::unique_ptr<FakeServer> fakeServer;
        try {
            fakeServer = std::make_unique<FakeServer>();
        } catch (const FakeServerException &e) {
            qWarning("Fake server exception: %s", e.what());
            QFAIL("Caught server exception");
        }

        TestableKDEDBolt kded(nullptr, {});
        QSignalSpy notifySpy(&kded, &TestableKDEDBolt::deviceNotify);
        QVERIFY(notifySpy.isValid());

        // Add unauthorized device
        auto fakeManager = fakeServer->manager();
        try {
            auto fakeDevice = std::make_unique<FakeDevice>(QStringLiteral("Device1"));
            fakeDevice->setStatus(QStringLiteral("connected"));
            fakeDevice->setAuthFlags(QStringLiteral("none"));
            fakeManager->addDevice(std::move(fakeDevice));

            QTRY_COMPARE(notifySpy.size(), 1);
            const auto devices = notifySpy[0][0].value<QList<QSharedPointer<Bolt::Device>>>();
            QCOMPARE(devices.size(), 1);
            const auto device = devices.front();
            QCOMPARE(device->uid(), QStringLiteral("Device1"));
            QCOMPARE(device->authFlags(), Bolt::Auth::None);
            QCOMPARE(device->status(), Bolt::Status::Connected);
        } catch (const FakeDeviceException &e) {
            qWarning("Fake device exception: %s", e.what());
            QFAIL("Caught device exception");
        }

        // Add authorized device
        notifySpy.clear();
        try {
            auto fakeDevice = std::make_unique<FakeDevice>(QStringLiteral("Device2"));
            fakeDevice->setStatus(QStringLiteral("authorized"));
            fakeDevice->setAuthFlags(QStringLiteral("nokey | boot"));
            fakeManager->addDevice(std::move(fakeDevice));

            QTest::qWait(200);
            QVERIFY(notifySpy.empty());
        } catch (const FakeDeviceException &e) {
            qWarning("Fake device exception: %s", e.what());
            QFAIL("Caught device exception");
        }
    }
};

QTEST_MAIN(KDEDTest)

#include "kdedtest.moc"
