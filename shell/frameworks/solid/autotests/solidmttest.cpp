/*
    SPDX-FileCopyrightText: 2008 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include <QObject>
#include <QThread>
#include <QThreadPool>
#include <qtconcurrentrun.h>

#include <QTest>

#include <solid/device.h>
#include <solid/genericinterface.h>
#include <solid/predicate.h>
#include <solid/storagedrive.h>
#include <solid/storagevolume.h>

class SolidMtTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testWorkerThread();
    void testThreadedPredicate();
    void testTextPredicates_data();
    void testTextPredicates();
    void testTextPredicatesExtended();
    void testDeviceMatching();
};

class WorkerThread : public QThread
{
    Q_OBJECT
protected:
    void run() override
    {
        Solid::Device dev("/org/freedesktop/Hal/devices/computer");

        const QList<Solid::Device> driveList = Solid::Device::listFromType(Solid::DeviceInterface::StorageDrive);
        for (const Solid::Device &solidDevice : driveList) {
            const Solid::StorageDrive *solidDrive = solidDevice.as<Solid::StorageDrive>();
            Q_ASSERT(solidDrive);
            Q_UNUSED(solidDrive);
        }
    }
};

static void doPredicates()
{
    Solid::Predicate p5 =
        Solid::Predicate::fromString("[[Processor.maxSpeed == 3201 AND Processor.canChangeFrequency == false] OR StorageVolume.mountPoint == '/media/blup']");

    Solid::Predicate p6 = Solid::Predicate::fromString("StorageVolume.usage == 'Other'");
    Solid::Predicate p7 = Solid::Predicate::fromString(QString("StorageVolume.usage == %1").arg((int)Solid::StorageVolume::Other));
    Solid::Predicate p8 = Solid::Predicate::fromString("StorageVolume.ignored == false");
}

QTEST_MAIN(SolidMtTest)

void SolidMtTest::testWorkerThread()
{
    Solid::Device dev("/org/freedesktop/Hal/devices/acpi_ADP1");

    WorkerThread *wt = new WorkerThread;
    wt->start();
    wt->wait();

    const QList<Solid::Device> driveList = Solid::Device::listFromType(Solid::DeviceInterface::StorageDrive);
    for (const Solid::Device &solidDevice : driveList) {
        const Solid::GenericInterface *solidDrive = solidDevice.as<Solid::GenericInterface>();
        Q_ASSERT(solidDrive);
        Q_UNUSED(solidDrive);
    }

    delete wt;
}

void SolidMtTest::testThreadedPredicate()
{
    QThreadPool::globalInstance()->setMaxThreadCount(10);
    const QList<QFuture<void>> futures = {
        QtConcurrent::run(&doPredicates),
        QtConcurrent::run(&doPredicates),
        QtConcurrent::run(&doPredicates),
        QtConcurrent::run(&doPredicates),
        QtConcurrent::run(&doPredicates),
        QtConcurrent::run(&doPredicates),
        QtConcurrent::run(&doPredicates),
        QtConcurrent::run(&doPredicates),
    };
    for (QFuture<void> f : futures) {
        f.waitForFinished();
    }
    QThreadPool::globalInstance()->setMaxThreadCount(1); // delete those threads
}


void SolidMtTest::testTextPredicates_data()
{
    QTest::addColumn<QString>("pstring");
    QTest::addColumn<bool>("isValidPredicate");

    QTest::newRow("empty") << QString() << false;
    QTest::newRow("simple") << QStringLiteral("@") << true;
    QTest::newRow("and") << QStringLiteral("@ AND @") << false;
    QTest::newRow("[and]") << QStringLiteral("[@ AND @]") << true;
    QTest::newRow("[3and]") << QStringLiteral("[@ AND @ AND @]") << false;
    QTest::newRow("[[and]and]") << QStringLiteral("[[@ AND @] AND @]") << true;
    QTest::newRow("[and[and]]") << QStringLiteral("[@ AND [@ AND @]]") << true;
    QTest::newRow("[[and][and] (unbalance)") << QStringLiteral("[ [@ AND @] AND [@ AND @]") << false;
    QTest::newRow("[[and][and]]") << QStringLiteral("[ [@ AND @] AND [@ AND @] ]") << true;
    QTest::newRow("[[[and]and]and]") << QStringLiteral("[ [ [ @ AND @] AND @ ] AND @]") << true;
}

/** @brief Check validity (and hence, parser-code) for a bunch of text predicates
 *
 */
void SolidMtTest::testTextPredicates()
{
    QFETCH(QString, pstring);
    QFETCH(bool, isValidPredicate);

    // The expressions contain @ as a placeholder, so they are
    // short enough to easily see the structure of the expression
    // in _data(); replace all those @s with an actual atom.
    const QString atom("StorageVolume.ignored == false");
    while(pstring.contains('@'))
    {
        pstring = pstring.replace('@', atom);
    }

    Solid::Predicate p = Solid::Predicate::fromString(pstring);
    QCOMPARE(p.isValid(), isValidPredicate);
}

void SolidMtTest::testTextPredicatesExtended()
{
    Solid::Predicate p = Solid::Predicate::fromString("[[StorageVolume.ignored == false AND StorageVolume.usage == 'FileSystem'] AND [StorageVolume.ignored == false AND StorageDrive.removable == true]]");
    QVERIFY(p.isValid());
    QCOMPARE(p.type(), Solid::Predicate::Conjunction);
    QCOMPARE(p.firstOperand().type(), Solid::Predicate::Conjunction);
    QCOMPARE(p.firstOperand().firstOperand().type(), Solid::Predicate::PropertyCheck);
    QCOMPARE(p.secondOperand().secondOperand().type(), Solid::Predicate::PropertyCheck);

    QCOMPARE(p.firstOperand().firstOperand().propertyName(), QStringLiteral("ignored"));
    QCOMPARE(p.firstOperand().firstOperand().interfaceType(), Solid::DeviceInterface::StorageVolume);
    // Let's not call firstOperand() quite so much, and copy into a local atom
    {
        auto atom = p.firstOperand().firstOperand();
        QVERIFY(atom.isValid());
        QCOMPARE(atom.propertyName(), QStringLiteral("ignored"));
        QCOMPARE(atom.matchingValue().userType(), QMetaType::Bool);
        QCOMPARE(atom.matchingValue().toBool(), false);
    }
    {
        auto atom = p.firstOperand().secondOperand();
        QVERIFY(atom.isValid());
        QCOMPARE(atom.interfaceType(), Solid::DeviceInterface::StorageVolume);
        QCOMPARE(atom.propertyName(), QStringLiteral("usage"));
        QCOMPARE(atom.matchingValue().userType(), QMetaType::QString);
        QCOMPARE(atom.matchingValue().toString(), QStringLiteral("FileSystem"));
    }
    {
        auto atom = p.secondOperand().firstOperand();
        QVERIFY(atom.isValid());
        QCOMPARE(atom.interfaceType(), Solid::DeviceInterface::StorageVolume);
        QCOMPARE(atom.propertyName(), QStringLiteral("ignored"));
        QCOMPARE(atom.matchingValue().userType(), QMetaType::Bool);
        QCOMPARE(atom.matchingValue().toBool(), false);
    }
    {
        auto atom = p.secondOperand().secondOperand();
        QVERIFY(atom.isValid());
        QCOMPARE(atom.interfaceType(), Solid::DeviceInterface::StorageDrive);
        QCOMPARE(atom.propertyName(), QStringLiteral("removable"));
        QCOMPARE(atom.matchingValue().userType(), QMetaType::Bool);
        QCOMPARE(atom.matchingValue().toBool(), true);
    }
}

void SolidMtTest::testDeviceMatching()
{
    auto deviceList = Solid::Device::allDevices();
    QVERIFY(!deviceList.isEmpty());

    Solid::Predicate volumesPredicate = Solid::Predicate::fromString("StorageVolume.ignored == false");
    QVERIFY(volumesPredicate.isValid());

    int volumes = 0;
    for (const auto &device : std::as_const(deviceList)) {
        if(volumesPredicate.matches(device))
        {
            volumes++;
        }
    }
    // If there are no removable volumes, that's fine; below we will be matching empty lists.
    // QVERIFY(volumes > 0);

    // From the pre-parsed query
    {
        auto matchedDeviceList = Solid::Device::listFromQuery(volumesPredicate);
        QCOMPARE(matchedDeviceList.size(), volumes);
    }
    // Same from text again
    {
        auto matchedDeviceList = Solid::Device::listFromQuery("StorageVolume.ignored == false");
        QCOMPARE(matchedDeviceList.size(), volumes);
    }

    // Are there removables? Hope so.
    //
    // The removableFSVolumesPredicate is checking two incompatible things:
    // a given Solid::Device isn't a Volume and a Drive at the same time,
    // so it cannot match.
    {
        Solid::Predicate fsVolumesPredicate = Solid::Predicate::fromString("[StorageVolume.ignored == false AND StorageVolume.usage == 'FileSystem']");
        Solid::Predicate removablePredicate = Solid::Predicate::fromString("StorageDrive.removable == true");
        Solid::Predicate removableFSVolumesPredicate = Solid::Predicate::fromString("[[StorageVolume.ignored == false AND StorageVolume.usage == 'FileSystem'] AND StorageDrive.removable == true]");
        volumes = 0;
        for (const auto &device : std::as_const(deviceList)) {
            qDebug() << device.displayName() << "fs?" << fsVolumesPredicate.matches(device) << "removable?" << removablePredicate.matches(device) << "removableFS?" << removableFSVolumesPredicate.matches(device);
            if (removableFSVolumesPredicate.matches(device))
            {
                volumes++;
            }
        }
        QCOMPARE(volumes, 0);

        auto matchedDeviceList = Solid::Device::listFromQuery(removableFSVolumesPredicate);
        QCOMPARE(matchedDeviceList.size(), volumes);
    }

    // Internally, the predicate is going to check interfaces,
    // so let's do that here as well. This will show no device is
    // both a Volume and a Drive.
    {
        const Solid::DeviceInterface::Type storageVolume = Solid::DeviceInterface::Type::StorageVolume;
        const Solid::DeviceInterface::Type storageDrive = Solid::DeviceInterface::Type::StorageDrive;

        int volumeCount = 0;
        int driveCount = 0;
        int bothCount = 0;

        for (const auto &device : std::as_const(deviceList)) {
            // Copied from internals in predicate.cpp:
            //        const DeviceInterface *iface = device.asDeviceInterface(d->ifaceType);
            const Solid::DeviceInterface *volumeIface = device.asDeviceInterface(storageVolume);
            const Solid::DeviceInterface *driveIface = device.asDeviceInterface(storageDrive);
            volumeCount += volumeIface ? 1 : 0;
            driveCount += driveIface ? 1 : 0;
            bothCount += (volumeIface && driveIface) ? 1 : 0;
        }
        QCOMPARE(bothCount, 0);
        if (volumeCount + driveCount > 0)
        {
            // On Linux CI, there is no DBus connection, so no drives or volumes at all.
            // On a local machine, or on FreeBSD CI, there are volumes and/or drives,
            // so verify that there's at least one of each.
            QVERIFY(volumeCount > 0);
            QVERIFY(driveCount > 0);
        }
    }
}

#include "solidmttest.moc"
