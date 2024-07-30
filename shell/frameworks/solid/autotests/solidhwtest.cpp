/*
    SPDX-FileCopyrightText: 2005 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include <QSignalSpy>
#include <QTest>

#include "solid/devices/managerbase_p.h"
#include <solid/device.h>
#include <solid/devicenotifier.h>
#include <solid/genericinterface.h>
#include <solid/predicate.h>
#include <solid/processor.h>
#include <solid/storageaccess.h>
#include <solid/storagevolume.h>

#include <fakedevice.h>
#include <fakemanager.h>

#include <stdlib.h>

#ifndef FAKE_COMPUTER_XML
#error "FAKE_COMPUTER_XML not set. An XML file describing a computer is required for this test"
#endif

class SolidHwTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void testAllDevices();
    void testDeviceBasicFeatures();
    void testManagerSignals();
    void testDeviceSignals();
    void testDeviceExistence();
    void testDeviceInterfaceIntrospection_data();
    void testDeviceInterfaceIntrospection();
    void testDeviceInterfaceIntrospectionCornerCases();
    void testDeviceInterfaces();
    void testInvalidPredicate();
    void testPredicate();
    void testQueryStorageVolumeOrProcessor();
    void testQueryStorageVolumeOrStorageAccess();
    void testQueryWithParentUdi();
    void testListFromTypeProcessor();
    void testListFromTypeInvalid();
    void testSetupTeardown();
    void testStorageAccessFromPath();
    void testStorageAccessFromPath_data();

private:
    Solid::Backends::Fake::FakeManager *fakeManager;
};

QTEST_MAIN(SolidHwTest)

void SolidHwTest::initTestCase()
{
    qputenv("SOLID_FAKEHW", FAKE_COMPUTER_XML);
    Solid::ManagerBasePrivate *manager = dynamic_cast<Solid::ManagerBasePrivate *>(Solid::DeviceNotifier::instance());
    fakeManager = qobject_cast<Solid::Backends::Fake::FakeManager *>(manager->managerBackends().first());
}

void SolidHwTest::testAllDevices()
{
    const QList<Solid::Device> devices = Solid::Device::allDevices();

    // Verify that the framework reported correctly the devices available
    // in the backend.
    QStringList expected_udis;
    QStringList received_udis;

    expected_udis = fakeManager->allDevices();
    std::sort(expected_udis.begin(), expected_udis.end());

    for (const Solid::Device &dev : devices) {
        received_udis << dev.udi();
    }

    std::sort(received_udis.begin(), received_udis.end());

    QCOMPARE(expected_udis, received_udis);
}

void SolidHwTest::testDeviceBasicFeatures()
{
    // Retrieve a valid Device object
    Solid::Device valid_dev("/org/kde/solid/fakehw/storage_model_solid_writer");

    QCOMPARE(valid_dev.isValid(), true);

    // A few attempts at creating invalid Device objects
    Solid::Device invalid_dev("uhoh? doesn't exist, I guess");
    QCOMPARE(invalid_dev.isValid(), false);
    invalid_dev = Solid::Device(QString());
    QCOMPARE(invalid_dev.isValid(), false);
    invalid_dev = Solid::Device();
    QCOMPARE(invalid_dev.isValid(), false);

    QCOMPARE(valid_dev.udi(), QString("/org/kde/solid/fakehw/storage_model_solid_writer"));
    QCOMPARE(invalid_dev.udi(), QString());

    // Query properties
    QCOMPARE(valid_dev.as<Solid::GenericInterface>()->propertyExists("name"), true);
    QCOMPARE(valid_dev.as<Solid::GenericInterface>()->propertyExists("foo.bar"), false);
    QCOMPARE((QObject *)invalid_dev.as<Solid::GenericInterface>(), (QObject *)nullptr);

    QCOMPARE(valid_dev.as<Solid::GenericInterface>()->property("name"), QVariant("Solid IDE DVD Writer"));
    QVERIFY(!valid_dev.as<Solid::GenericInterface>()->property("foo.bar").isValid());

    Solid::Backends::Fake::FakeDevice *fake_device = fakeManager->findDevice("/org/kde/solid/fakehw/storage_model_solid_writer");
    QMap<QString, QVariant> expected_properties = fake_device->allProperties();

    QCOMPARE(valid_dev.as<Solid::GenericInterface>()->allProperties(), expected_properties);

    // Query device interfaces
    QCOMPARE(valid_dev.isDeviceInterface(Solid::DeviceInterface::StorageDrive), true);
    QCOMPARE(valid_dev.isDeviceInterface(Solid::DeviceInterface::OpticalDrive), true);
    QCOMPARE(valid_dev.isDeviceInterface(Solid::DeviceInterface::StorageVolume), false);

    QCOMPARE(invalid_dev.isDeviceInterface(Solid::DeviceInterface::Unknown), false);
    QCOMPARE(invalid_dev.isDeviceInterface(Solid::DeviceInterface::StorageDrive), false);

    // Query parent
    QCOMPARE(valid_dev.parentUdi(), QString("/org/kde/solid/fakehw/pci_002_ide_1_0"));
    QCOMPARE(valid_dev.parent().udi(), Solid::Device("/org/kde/solid/fakehw/pci_002_ide_1_0").udi());

    QVERIFY(!invalid_dev.parent().isValid());
    QVERIFY(invalid_dev.parentUdi().isEmpty());

    // Query vendor/product
    QCOMPARE(valid_dev.vendor(), QString("Acme Corporation"));
    QCOMPARE(valid_dev.product(), QString("Solid IDE DVD Writer"));

    QCOMPARE(invalid_dev.vendor(), QString());
    QCOMPARE(invalid_dev.product(), QString());
}

void SolidHwTest::testManagerSignals()
{
    fakeManager->unplug("/org/kde/solid/fakehw/acpi_CPU0");

    // Heh, we missed a processor in this system ;-)
    // We're going to add this device, and check that the signal has been
    // properly emitted by the manager
    QSignalSpy added(Solid::DeviceNotifier::instance(), SIGNAL(deviceAdded(QString)));
    fakeManager->plug("/org/kde/solid/fakehw/acpi_CPU0");
    QCOMPARE(added.count(), 1);
    QCOMPARE(added.at(0).at(0).toString(), QString("/org/kde/solid/fakehw/acpi_CPU0"));

    // Moreover we check that the device is really available
    Solid::Device cpu("/org/kde/solid/fakehw/acpi_CPU0");
    QVERIFY(cpu.isValid());

    // Finally we remove the device and spy the corresponding signal again
    QSignalSpy removed(Solid::DeviceNotifier::instance(), SIGNAL(deviceRemoved(QString)));
    fakeManager->unplug("/org/kde/solid/fakehw/acpi_CPU0");
    QCOMPARE(added.count(), 1);
    QCOMPARE(added.at(0).at(0).toString(), QString("/org/kde/solid/fakehw/acpi_CPU0"));

    // The Device object should become automatically invalid
    QVERIFY(!cpu.isValid());

    // Restore original state
    fakeManager->plug("/org/kde/solid/fakehw/acpi_CPU0");
}

void SolidHwTest::testDeviceSignals()
{
    // A button is a nice device for testing state changes, isn't it?
    Solid::Backends::Fake::FakeDevice *fake = fakeManager->findDevice("/org/kde/solid/fakehw/platform_floppy_0_storage_virt_volume");
    Solid::Device device("/org/kde/solid/fakehw/platform_floppy_0_storage_virt_volume");

    // We'll spy our floppy
    QSignalSpy condition_raised(device.as<Solid::GenericInterface>(), SIGNAL(conditionRaised(QString, QString)));
    QSignalSpy property_changed(device.as<Solid::GenericInterface>(), SIGNAL(propertyChanged(QMap<QString, int>)));

    fake->setProperty("mountPoint", "/tmp.foo"); // The button is now pressed (modified property)
    fake->raiseCondition("Floppy Closed", "Why not?"); // Since it's a LID we notify this change
    fake->setProperty("hactar", 42); // We add a property
    fake->removeProperty("hactar"); // We remove a property

    // 3 property changes occurred in the device
    QCOMPARE(property_changed.count(), 3);

    QMap<QString, int> changes;

    // First one is a "PropertyModified" for "button.state"
    changes = property_changed.at(0).at(0).value<QMap<QString, int>>();
    QCOMPARE(changes.count(), 1);
    QVERIFY(changes.contains("mountPoint"));
    QCOMPARE(changes["mountPoint"], (int)Solid::GenericInterface::PropertyModified);

    // Second one is a "PropertyAdded" for "hactar"
    changes = property_changed.at(1).at(0).value<QMap<QString, int>>();
    QCOMPARE(changes.count(), 1);
    QVERIFY(changes.contains("hactar"));
    QCOMPARE(changes["hactar"], (int)Solid::GenericInterface::PropertyAdded);

    // Third one is a "PropertyRemoved" for "hactar"
    changes = property_changed.at(2).at(0).value<QMap<QString, int>>();
    QCOMPARE(changes.count(), 1);
    QVERIFY(changes.contains("hactar"));
    QCOMPARE(changes["hactar"], (int)Solid::GenericInterface::PropertyRemoved);

    // Only one condition has been raised in the device
    QCOMPARE(condition_raised.count(), 1);

    // It must be identical to the condition we raised by hand
    QCOMPARE(condition_raised.at(0).at(0).toString(), QString("Floppy Closed"));
    QCOMPARE(condition_raised.at(0).at(1).toString(), QString("Why not?"));
}

void SolidHwTest::testDeviceExistence()
{
    QCOMPARE(Solid::Device("/org/kde/solid/fakehw/platform_floppy_0_storage_virt_volume").isValid(), true);
    QCOMPARE(Solid::Device("/org/kde/solid/fakehw/volume_label_SOLIDMAN_BEGINS").isValid(), true);

    // Note the extra space
    QCOMPARE(Solid::Device("/org/kde/solid/fakehw/computer ").isValid(), false);
    QCOMPARE(Solid::Device("#'({(]").isValid(), false);
    QCOMPARE(Solid::Device(QString()).isValid(), false);

    // Now try to see if isValid() changes on plug/unplug events
    Solid::Device cpu("/org/kde/solid/fakehw/acpi_CPU0");
    QVERIFY(cpu.isValid());
    fakeManager->unplug("/org/kde/solid/fakehw/acpi_CPU0");
    QVERIFY(!cpu.isValid());
    fakeManager->plug("/org/kde/solid/fakehw/acpi_CPU0");
    QVERIFY(cpu.isValid());
}

void SolidHwTest::testDeviceInterfaces()
{
    Solid::Device cpu("/org/kde/solid/fakehw/acpi_CPU0");

    Solid::DeviceInterface *iface = cpu.asDeviceInterface(Solid::DeviceInterface::Processor);
    Solid::DeviceInterface *processor = cpu.as<Solid::Processor>();

    QVERIFY(cpu.isDeviceInterface(Solid::DeviceInterface::Processor));
    QVERIFY(iface != nullptr);
    QCOMPARE(iface, processor);

    Solid::Device cpu2("/org/kde/solid/fakehw/acpi_CPU0");
    QCOMPARE(cpu.as<Solid::Processor>(), cpu2.as<Solid::Processor>());
    QCOMPARE(cpu.as<Solid::GenericInterface>(), cpu2.as<Solid::GenericInterface>());

    QPointer<Solid::Processor> p = cpu.as<Solid::Processor>();
    QVERIFY(p != nullptr);
    fakeManager->unplug("/org/kde/solid/fakehw/acpi_CPU0");
    QVERIFY(p == nullptr);
    fakeManager->plug("/org/kde/solid/fakehw/acpi_CPU0");

    QPointer<Solid::StorageVolume> v;
    QPointer<Solid::StorageVolume> v2;
    {
        Solid::Device partition("/org/kde/solid/fakehw/volume_uuid_f00ba7");
        v = partition.as<Solid::StorageVolume>();
        QVERIFY(v != nullptr);
        {
            Solid::Device partition2("/org/kde/solid/fakehw/volume_uuid_f00ba7");
            v2 = partition2.as<Solid::StorageVolume>();
            QVERIFY(v2 != nullptr);
            QVERIFY(v == v2);
        }
        QVERIFY(v != nullptr);
        QVERIFY(v2 != nullptr);
    }
    QVERIFY(v != nullptr);
    QVERIFY(v2 != nullptr);
    fakeManager->unplug("/org/kde/solid/fakehw/volume_uuid_f00ba7");
    QVERIFY(v == nullptr);
    QVERIFY(v2 == nullptr);
    fakeManager->plug("/org/kde/solid/fakehw/volume_uuid_f00ba7");
}

void SolidHwTest::testDeviceInterfaceIntrospection_data()
{
    QTest::addColumn<QString>("name");
    QTest::addColumn<int>("value");

    QTest::newRow("DeviceInterface: Unknown") << "Unknown" << (int)Solid::DeviceInterface::Unknown;
    QTest::newRow("DeviceInterface: Processor") << "Processor" << (int)Solid::DeviceInterface::Processor;
    QTest::newRow("DeviceInterface: Block") << "Block" << (int)Solid::DeviceInterface::Block;
    QTest::newRow("DeviceInterface: StorageDrive") << "StorageDrive" << (int)Solid::DeviceInterface::StorageDrive;
    QTest::newRow("DeviceInterface: OpticalDrive") << "OpticalDrive" << (int)Solid::DeviceInterface::OpticalDrive;
    QTest::newRow("DeviceInterface: StorageVolume") << "StorageVolume" << (int)Solid::DeviceInterface::StorageVolume;
    QTest::newRow("DeviceInterface: OpticalDisc") << "OpticalDisc" << (int)Solid::DeviceInterface::OpticalDisc;
    QTest::newRow("DeviceInterface: Camera") << "Camera" << (int)Solid::DeviceInterface::Camera;
    QTest::newRow("DeviceInterface: PortableMediaPlayer") << "PortableMediaPlayer" << (int)Solid::DeviceInterface::PortableMediaPlayer;
    QTest::newRow("DeviceInterface: Battery") << "Battery" << (int)Solid::DeviceInterface::Battery;
}

void SolidHwTest::testDeviceInterfaceIntrospection()
{
    QFETCH(QString, name);
    QFETCH(int, value);

    QCOMPARE(Solid::DeviceInterface::typeToString((Solid::DeviceInterface::Type)value), name);
    QCOMPARE((int)Solid::DeviceInterface::stringToType(name), value);
}

void SolidHwTest::testDeviceInterfaceIntrospectionCornerCases()
{
    QCOMPARE(Solid::DeviceInterface::typeToString((Solid::DeviceInterface::Type)-1), QString());
    QCOMPARE((int)Solid::DeviceInterface::stringToType("blup"), -1);
}

static QStringList to_string_list(const QList<Solid::Device> &list)
{
    QStringList res;
    res.reserve(list.size());
    for (const Solid::Device &device : list) {
        res << device.udi();
    }
    return res;
}

void SolidHwTest::testInvalidPredicate()
{
    QString str_pred = "[[Processor.maxSpeed == 3201 AND Processor.canChangeFrequency == false] OR StorageVolume.mountPoint == '/media/blup']";
    // Since str_pred is canonicalized, fromString().toString() should be invariant
    QCOMPARE(Solid::Predicate::fromString(str_pred).toString(), str_pred);

    // Invalid predicate
    str_pred = "[StorageVolume.ignored == false AND OpticalDisc.isBlank == true AND OpticalDisc.discType & 'CdRecordable|CdRewritable']";
    QVERIFY(!Solid::Predicate::fromString(str_pred).isValid());
}

void SolidHwTest::testPredicate()
{
    Solid::Device dev("/org/kde/solid/fakehw/acpi_CPU0");
    /* clang-format off */
    Solid::Predicate p1 = (Solid::Predicate(Solid::DeviceInterface::Processor, "maxSpeed", 3200)
                           & Solid::Predicate(Solid::DeviceInterface::Processor, "canChangeFrequency", true));
    Solid::Predicate p2 = Solid::Predicate(Solid::DeviceInterface::Processor, "maxSpeed", 3200)
                          & Solid::Predicate(Solid::DeviceInterface::Processor, "canChangeFrequency", false);
    Solid::Predicate p3 = Solid::Predicate(Solid::DeviceInterface::Processor, "maxSpeed", 3201)
                          | Solid::Predicate(Solid::DeviceInterface::Processor, "canChangeFrequency", true);
    Solid::Predicate p4 = Solid::Predicate(Solid::DeviceInterface::Processor, "maxSpeed", 3201)
                          | Solid::Predicate(Solid::DeviceInterface::Processor, "canChangeFrequency", false);
    Solid::Predicate p5 =
        Solid::Predicate::fromString("[[Processor.maxSpeed == 3201 AND Processor.canChangeFrequency == false] OR StorageVolume.mountPoint == '/media/blup']");
    /* clang-format on */

    QVERIFY(p1.matches(dev));
    QVERIFY(!p2.matches(dev));
    QVERIFY(p3.matches(dev));
    QVERIFY(!p4.matches(dev));

    Solid::Predicate p6 = Solid::Predicate::fromString("StorageVolume.usage == 'Other'");
    Solid::Predicate p7 = Solid::Predicate::fromString(QString("StorageVolume.usage == %1").arg((int)Solid::StorageVolume::Other));
    QVERIFY(!p6.matches(dev));
    QVERIFY(!p7.matches(dev));
    dev = Solid::Device("/org/kde/solid/fakehw/volume_part2_size_1024");
    QVERIFY(p6.matches(dev));
    QVERIFY(p7.matches(dev));

    QList<Solid::Device> list;

    QStringList cpuSet;
    cpuSet << QString("/org/kde/solid/fakehw/acpi_CPU0") << QString("/org/kde/solid/fakehw/acpi_CPU1");

    list = Solid::Device::listFromQuery(p1);
    QCOMPARE(list.size(), 2);
    QCOMPARE(to_string_list(list), cpuSet);

    list = Solid::Device::listFromQuery(p2);
    QCOMPARE(list.size(), 0);

    list = Solid::Device::listFromQuery(p3);
    QCOMPARE(list.size(), 2);
    QCOMPARE(to_string_list(list), cpuSet);

    list = Solid::Device::listFromQuery(p4);
    QCOMPARE(list.size(), 0);

    list = Solid::Device::listFromQuery("[Processor.canChangeFrequency==true AND Processor.number==1]");
    QCOMPARE(list.size(), 1);
    QCOMPARE(list.at(0).udi(), QString("/org/kde/solid/fakehw/acpi_CPU1"));

    // writeSpeeds is a QList, make sure we can match a single element.
    list = Solid::Device::listFromQuery("[OpticalDrive.writeSpeeds==2117 AND OpticalDrive.removable==true]");
    QCOMPARE(list.size(), 1);
    QCOMPARE(list.at(0).udi(), QString("/org/kde/solid/fakehw/storage_model_solid_writer"));
}

void SolidHwTest::testQueryStorageVolumeOrProcessor()
{
    auto list = Solid::Device::listFromQuery("[Processor.number==1 OR IS StorageVolume]");
    QCOMPARE(list.size(), 13);

    // make sure predicate case-insensitiveness is sane
    list = Solid::Device::listFromQuery("[Processor.number==1 or is StorageVolume]");
    QCOMPARE(list.size(), 13);
    list = Solid::Device::listFromQuery("[Processor.number==1 oR Is StorageVolume]");
    QCOMPARE(list.size(), 13);
    QStringList expected{"/org/kde/solid/fakehw/acpi_CPU1",
                         "/org/kde/solid/fakehw/platform_floppy_0_storage_virt_volume",
                         "/org/kde/solid/fakehw/volume_0000_unmounted_storage",
                         "/org/kde/solid/fakehw/volume_label_SOLIDMAN_BEGINS",
                         "/org/kde/solid/fakehw/volume_part1_size_993284096",
                         "/org/kde/solid/fakehw/volume_part2_size_1024",
                         "/org/kde/solid/fakehw/volume_part5_size_1048576",
                         "/org/kde/solid/fakehw/volume_uuid_5011",
                         "/org/kde/solid/fakehw/volume_uuid_c0ffee",
                         "/org/kde/solid/fakehw/volume_uuid_cleartext_data_0123",
                         "/org/kde/solid/fakehw/volume_uuid_encrypted_0123",
                         "/org/kde/solid/fakehw/volume_uuid_f00ba7",
                         "/org/kde/solid/fakehw/volume_uuid_feedface"};
    QCOMPARE(to_string_list(list), expected);

    list = Solid::Device::listFromQuery("[IS Processor OR IS StorageVolume]");
    QCOMPARE(list.size(), 14);
    expected.prepend("/org/kde/solid/fakehw/acpi_CPU0");
    QCOMPARE(to_string_list(list), expected);
}

void SolidHwTest::testQueryStorageVolumeOrStorageAccess()
{
    // the query from KFilePlacesModel
    const auto list = Solid::Device::listFromQuery(
        "[[[[ StorageVolume.ignored == false AND [ StorageVolume.usage == 'FileSystem' OR StorageVolume.usage == 'Encrypted' ]]"
        " OR "
        "[ IS StorageAccess AND StorageDrive.driveType == 'Floppy' ]]"
        " OR "
        "OpticalDisc.availableContent & 'Audio' ]"
        " OR "
        "StorageAccess.ignored == false ]");
    const QStringList expected{"/org/kde/solid/fakehw/fstab/thehost/solidpath",
                               "/org/kde/solid/fakehw/platform_floppy_0_storage_virt_volume",
                               "/org/kde/solid/fakehw/volume_part1_size_993284096",
                               "/org/kde/solid/fakehw/volume_uuid_5011",
                               "/org/kde/solid/fakehw/volume_uuid_f00ba7"};
    QCOMPARE(to_string_list(list), expected);
}

void SolidHwTest::testQueryWithParentUdi()
{
    QString parentUdi = "/org/kde/solid/fakehw/storage_model_solid_reader";
    Solid::DeviceInterface::Type ifaceType = Solid::DeviceInterface::Unknown;
    QCOMPARE(fakeManager->devicesFromQuery(parentUdi, ifaceType).size(), 1);
    QCOMPARE(fakeManager->devicesFromQuery(parentUdi, ifaceType).at(0), QString("/org/kde/solid/fakehw/volume_label_SOLIDMAN_BEGINS"));

    ifaceType = Solid::DeviceInterface::Processor;
    QCOMPARE(fakeManager->devicesFromQuery(parentUdi, ifaceType).size(), 0);

    parentUdi = "/org/kde/solid/fakehw/computer";
    QCOMPARE(fakeManager->devicesFromQuery(parentUdi, ifaceType).size(), 2);
    QCOMPARE(fakeManager->devicesFromQuery(parentUdi, ifaceType).at(0), QString("/org/kde/solid/fakehw/acpi_CPU0"));
    QCOMPARE(fakeManager->devicesFromQuery(parentUdi, ifaceType).at(1), QString("/org/kde/solid/fakehw/acpi_CPU1"));
}

void SolidHwTest::testListFromTypeProcessor()
{
    const auto ifaceType = Solid::DeviceInterface::Processor;
    const auto list = Solid::Device::listFromType(ifaceType, QString());
    QCOMPARE(list.size(), 2);
    QCOMPARE(list.at(0).udi(), QString("/org/kde/solid/fakehw/acpi_CPU0"));
    QCOMPARE(list.at(1).udi(), QString("/org/kde/solid/fakehw/acpi_CPU1"));
}

void SolidHwTest::testListFromTypeInvalid()
{
    const auto list = Solid::Device::listFromQuery("blup", QString());
    QCOMPARE(list.size(), 0);
}

void SolidHwTest::testSetupTeardown()
{
    Solid::StorageAccess *access;
    {
        Solid::Device device("/org/kde/solid/fakehw/volume_part1_size_993284096");
        access = device.as<Solid::StorageAccess>();
    }

    QList<QVariant> args;
    QSignalSpy spy(access, SIGNAL(accessibilityChanged(bool, QString)));

    access->teardown();

    QCOMPARE(spy.count(), 1);
    args = spy.takeFirst();
    QCOMPARE(args.at(0).toBool(), false);

    access->setup();

    QCOMPARE(spy.count(), 1);
    args = spy.takeFirst();
    QCOMPARE(args.at(0).toBool(), true);
}

void SolidHwTest::testStorageAccessFromPath()
{
    QFETCH(QString, path);
    QFETCH(QString, deviceUdi);

    const auto prefix = QStringLiteral("/org/kde/solid/fakehw/");

    const Solid::Device device = Solid::Device::storageAccessFromPath(path);
#if defined(Q_OS_WIN)
    // storageAccessFromPath has never been working correctly on Windows
    // Fixing this is left for someone with access to a Windows machine
    return;
#endif

    QVERIFY(device.isValid());
    QCOMPARE(device.udi(), prefix + deviceUdi);

    auto storage = device.as<Solid::StorageAccess>();
    QVERIFY(storage);
    QVERIFY(!storage->filePath().isEmpty());
    QVERIFY(path.startsWith(storage->filePath()));
}

void SolidHwTest::testStorageAccessFromPath_data()
{
    QTest::addColumn<QString>("path");
    QTest::addColumn<QString>("deviceUdi");

    QTest::addRow("Root")            << QStringLiteral("/")           << QStringLiteral("volume_uuid_feedface");
    QTest::addRow("Mount directory") << QStringLiteral("/media")      << QStringLiteral("volume_uuid_feedface");
    QTest::addRow("NFS mount")       << QStringLiteral("/media/nfs")  << QStringLiteral("fstab/thehost/solidpath");
    QTest::addRow("Home directory")  << QStringLiteral("/home")       << QStringLiteral("volume_uuid_c0ffee");
    QTest::addRow("Other home")      << QStringLiteral("/home_other") << QStringLiteral("volume_uuid_feedface");
    QTest::addRow("User home")       << QStringLiteral("/home/user")  << QStringLiteral("volume_uuid_c0ffee");
    QTest::addRow("LUKS")            << QStringLiteral("/data")       << QStringLiteral("volume_uuid_cleartext_data_0123");
}

#include "solidhwtest.moc"
