/*
 * SPDX-FileCopyrightText: 2015 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

import QtTest 1.0
import QtQuick 2.2
import org.kde.bluezqt.fakebluez 1.0
import org.kde.bluezqt 1.0 as BluezQt

TestCase {
    name: "Manager"

    readonly property BluezQt.Manager manager: BluezQt.Manager

    function cleanup()
    {
        FakeBluez.stop();
    }

    function test_bluezNotRunning()
    {
        tryCompare(manager, "operational", false);

        verify(!manager.operational, "operational");
        verify(!manager.bluetoothOperational, "btOperational");
    }

    function test_bluezNotExportingInterfaces()
    {
        FakeBluez.start();
        FakeBluez.runTest("bluez-not-exporting-interfaces");

        tryCompare(manager, "operational", false);

        verify(!manager.operational, "operational");
        verify(!manager.bluetoothOperational, "btOperational");
    }

    function test_bluezNoAdapters()
    {
        FakeBluez.start();
        FakeBluez.runTest("bluez-no-adapters");

        tryCompare(manager, "operational", true);

        verify(manager.operational, "operational");
        verify(!manager.bluetoothOperational, "btOperational");
    }

    SignalSpy {
        id: usableAdapterChangedSpy
        signalName: "usableAdapterChanged"
    }

    function test_usableAdapter()
    {
        FakeBluez.start();
        FakeBluez.runTest("bluez-standard");

        var adapter1path = "/org/bluez/hci0";
        var adapter1props = {
            Path: adapter1path,
            Address: "1C:E5:C3:BC:94:7E",
            Name: "TestAdapter",
            Powered: false,
            _toDBusObjectPath: [ "Path" ]
        }
        FakeBluez.runAction("devicemanager", "create-adapter", adapter1props);

        var adapter2path = "/org/bluez/hci1";
        var adapter2props = {
            Path: adapter2path,
            Address: "2E:3A:C3:BC:85:7C",
            Name: "TestAdapter2",
            Powered: false,
            _toDBusObjectPath: [ "Path" ]
        }
        FakeBluez.runAction("devicemanager", "create-adapter", adapter2props);


        tryCompare(manager, "operational", true);
        verify(!manager.bluetoothOperational, "btOperational");

        usableAdapterChangedSpy.target = manager;

        var properties = {
            Path: adapter1path,
            Name: "Powered",
            Value: true,
            _toDBusObjectPath: [ "Path" ]
        }
        FakeBluez.runAction("devicemanager", "change-adapter-property", properties);

        tryCompare(usableAdapterChangedSpy, "count", 1);
        compare(manager.usableAdapter.ubi, adapter1path);

        usableAdapterChangedSpy.clear();

        properties.Value = false;
        FakeBluez.runAction("devicemanager", "change-adapter-property", properties);

        tryCompare(usableAdapterChangedSpy, "count", 1);
        compare(manager.usableAdapter, null);

        usableAdapterChangedSpy.clear();

        properties.Path = adapter2path;
        properties.Value = true;
        FakeBluez.runAction("devicemanager", "change-adapter-property", properties);

        tryCompare(usableAdapterChangedSpy, "count", 1);
        compare(manager.usableAdapter.ubi, adapter2path);
    }

    SignalSpy {
        id: managerAdapterRemovedSpy
        signalName: "adapterRemoved"
    }

    SignalSpy {
        id: managerDeviceRemovedSpy
        signalName: "deviceRemoved"
    }

    function test_adapterWithDevicesRemoved()
    {
        FakeBluez.start();
        FakeBluez.runTest("bluez-standard");

        var adapter1path = "/org/bluez/hci0";
        var adapterProps = {
            Path: adapter1path,
            Address: "1C:E5:C3:BC:94:7E",
            Name: "TestAdapter",
            Powered: false,
            _toDBusObjectPath: [ "Path" ]
        }
        FakeBluez.runAction("devicemanager", "create-adapter", adapterProps);

        var device1props = {
            Path: "/org/bluez/hci0/dev_40_79_6A_0C_39_75",
            Adapter: adapter1path,
            Address: "40:79:6A:0C:39:75",
            Name: "TestDevice",
            _toDBusObjectPath: [ "Path", "Adapter" ]
        }
        FakeBluez.runAction("devicemanager", "create-device", device1props);

        var device2props = {
            Path: "/org/bluez/hci0/dev_50_79_6A_0C_39_75",
            Adapter: adapter1path,
            Address: "50:79:6A:0C:39:75",
            Name: "TestDevice2",
            _toDBusObjectPath: [ "Path", "Adapter" ]
        }
        FakeBluez.runAction("devicemanager", "create-device", device2props);

        tryCompare(manager, "operational", true);

        var adapter = manager.adapterForAddress("1C:E5:C3:BC:94:7E");
        var device1 = manager.deviceForAddress("40:79:6A:0C:39:75");
        var device2 = manager.deviceForAddress("50:79:6A:0C:39:75");

        verify(adapter);
        verify(device1);
        verify(device2);

        managerAdapterRemovedSpy.target = manager;
        managerDeviceRemovedSpy.target = manager;

        var properties = {
            Path: adapter1path,
            _toDBusObjectPath: [ "Path" ]
        }
        FakeBluez.runAction("devicemanager", "remove-adapter", properties);

        tryCompare(managerAdapterRemovedSpy, "count", 1);
        tryCompare(managerDeviceRemovedSpy, "count", 2);
        compare(manager.adapters.length, 0);
        compare(manager.devices.length, 0);
    }
}
