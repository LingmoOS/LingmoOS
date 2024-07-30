/*
    SPDX-FileCopyrightText: 2006 Michaël Larouche <michael.larouche@kdemail.net>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#include "fakehardwaretest.h"

// Qt includes
#include <QTest>

// Solid includes
#include <solid/devices/ifaces/device.h>
#include <solid/devices/ifaces/deviceinterface.h>
#include <solid/devices/ifaces/processor.h>

// Local includes
#include "solid/devices/backends/fakehw/fakedevice.h"
#include "solid/devices/backends/fakehw/fakemanager.h"

QTEST_MAIN(FakeHardwareTest)

void FakeHardwareTest::testFakeBackend()
{
    Solid::Backends::Fake::FakeManager *fakeManager = new Solid::Backends::Fake::FakeManager(nullptr, TEST_DATA);

    QVERIFY(!fakeManager->allDevices().isEmpty());
    QObject *computer = fakeManager->createDevice("/org/kde/solid/fakehw/computer");
    QVERIFY(computer != nullptr);
    QVERIFY(fakeManager->createDevice("/com/helloworld/troll/compiutor") == nullptr);

    Solid::Backends::Fake::FakeDevice *device = static_cast<Solid::Backends::Fake::FakeDevice *>(fakeManager->createDevice("/org/kde/solid/fakehw/acpi_CPU0"));

    QCOMPARE(device->udi(), QString("/org/kde/solid/fakehw/acpi_CPU0"));
    QCOMPARE(device->parentUdi(), QString("/org/kde/solid/fakehw/computer"));
    QCOMPARE(device->vendor(), QString("Acme Corporation"));
    QCOMPARE(device->product(), QString("Solid Processor #0"));

    QCOMPARE(device->property("number").toString(), QString("0"));
    QVERIFY(device->propertyExists("number"));
    QVERIFY(!device->propertyExists("youstfuqewerrernoob"));

    QVERIFY(device->queryDeviceInterface(Solid::DeviceInterface::Processor));

    QObject *interface = device->createDeviceInterface(Solid::DeviceInterface::Processor);
    Solid::Ifaces::Processor *processor = qobject_cast<Solid::Ifaces::Processor *>(interface);

    QCOMPARE(processor->number(), 0);
    QCOMPARE(processor->canChangeFrequency(), true);
    QCOMPARE((int)processor->maxSpeed(), 3200);

    Solid::Processor::InstructionSets instructionsets;
    instructionsets |= Solid::Processor::IntelMmx;
    instructionsets |= Solid::Processor::IntelSse;
    QCOMPARE(processor->instructionSets(), instructionsets);

    delete processor;
    delete device;
    delete computer;
    delete fakeManager;
}

#include "moc_fakehardwaretest.cpp"
