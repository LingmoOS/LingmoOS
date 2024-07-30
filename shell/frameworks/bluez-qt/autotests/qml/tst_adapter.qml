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
    name: "Adapter"

    readonly property BluezQt.Manager manager: BluezQt.Manager

    property var adapter1props
    property var adapter2props

    function initTestCase()
    {
        FakeBluez.start();
        FakeBluez.runTest("bluez-standard");

        // Create adapters
        var adapter1path = "/org/bluez/hci0";
        adapter1props = {
            Path: adapter1path,
            Address: "1C:E5:C3:BC:94:7E",
            Name: "TestAdapter",
            Alias: "TestAlias",
            Class: 101,
            Powered: false,
            Discoverable: false,
            Pairable: false,
            PairableTimeout: 0,
            DiscoverableTimeout: 0,
            Discovering: false,
            UUIDs: [ "00001200-0000-1000-8000-00805f9b34fb" ],
            Modalias: "usb:v2D6Bp1236d0215",
            _toDBusObjectPath: [ "Path" ]
        }
        FakeBluez.runAction("devicemanager", "create-adapter", adapter1props);

        var adapter2path = "/org/bluez/hci1";
        adapter2props = {
            Path: adapter2path,
            Address: "2E:3A:C3:BC:85:7C",
            Name: "TestAdapter2",
            Alias: "TestAlias2",
            Class: 201,
            Powered: true,
            Discoverable: true,
            Pairable: true,
            PairableTimeout: 150,
            DiscoverableTimeout: 120,
            Discovering: false,
            UUIDs: [ "0000110c-0000-1000-8000-00805f9b34fb" ],
            Modalias: "usb:v1D3Bp1134d0214",
            _toDBusObjectPath: [ "Path" ]
        }
        FakeBluez.runAction("devicemanager", "create-adapter", adapter2props);

        // Create devices
        var device1props = {
            Path: "/org/bluez/hci0/dev_40_79_6A_0C_39_75",
            Adapter: adapter1path,
            Address: "40:79:6A:0C:39:75",
            Name: "TestDevice",
            _toDBusObjectPath: [ "Path", "Adapter" ]
        }
        FakeBluez.runAction("devicemanager", "create-device", device1props);

        var device2props = {
            Path: "/org/bluez/hci1/dev_50_79_6A_0C_39_75",
            Adapter: adapter2path,
            Address: "50:79:6A:0C:39:75",
            Name: "TestDevice2",
            Connected: true,
            _toDBusObjectPath: [ "Path", "Adapter" ]
        }
        FakeBluez.runAction("devicemanager", "create-device", device2props);

        tryCompare(manager, "operational", true);
        compare(manager.adapters.length, 2, "adapters-length");
        compare(manager.devices.length, 2, "devices-length");
        compare(manager.connectedDevices.length, 1, "connected-devices-length");
    }

    function cleanupTestCase()
    {
        FakeBluez.stop();
    }

    function compareProperties(adapter, props)
    {
        compare(adapter.ubi, props.Path, "ubi");
        compare(adapter.address, props.Address, "address");
        compare(adapter.name, props.Alias, "name");
        compare(adapter.systemName, props.Name, "systemName");
        compare(adapter.adapterClass, props.Class, "adapterClass");
        compare(adapter.powered, props.Powered, "powered");
        compare(adapter.discoverable, props.Discoverable, "discoverable");
        compare(adapter.pairable, props.Pairable, "pairable");
        compare(adapter.pairableTimeout, props.PairableTimeout, "pairableTimeout");
        compare(adapter.discoverableTimeout, props.DiscoverableTimeout, "discoverableTimeout");
        compare(adapter.discovering, props.Discovering, "discovering");
        compare(adapter.modalias, props.Modalias, "modalias");

        for (var i = 0; i < adapter.uuids.length; ++i) {
            compare(adapter.uuids[i], props.UUIDs[i].toUpperCase(), "uuids" + i);
        }
    }

    function test_getProperties()
    {
        var adapter1 = manager.adapterForUbi(adapter1props.Path);
        var adapter2 = manager.adapterForUbi(adapter2props.Path);
        compareProperties(adapter1, adapter1props);
        compareProperties(adapter2, adapter2props);
    }

    SignalSpy {
        id: nameChangedSpy
        signalName: "nameChanged"
    }

    function test_setName()
    {
        for (var i = 0; i < manager.adapters.length; ++i) {
            var adapter = manager.adapters[i];
            var value = adapter.name + "tst_name";

            nameChangedSpy.clear();
            nameChangedSpy.target = adapter;

            adapter.name = value;

            tryCompare(nameChangedSpy, "count", 1);
            compare(adapter.name, value);
        }
    }

    SignalSpy {
        id: poweredChangedSpy
        signalName: "poweredChanged"
    }

    function test_setPowered()
    {
        for (var i = 0; i < manager.adapters.length; ++i) {
            var adapter = manager.adapters[i];
            var value = !adapter.powered;

            poweredChangedSpy.clear();
            poweredChangedSpy.target = adapter;

            adapter.powered = value;

            tryCompare(poweredChangedSpy, "count", 1);
            compare(adapter.powered, value);
        }
    }

    SignalSpy {
        id: discoveringChangedSpy
        signalName: "discoveringChanged"
    }

    function test_discovery()
    {
        for (var i = 0; i < manager.adapters.length; ++i) {
            var adapter = manager.adapters[i];

            discoveringChangedSpy.clear();
            discoveringChangedSpy.target = adapter;

            adapter.startDiscovery();

            tryCompare(discoveringChangedSpy, "count", 1);
            compare(adapter.discovering, true);

            discoveringChangedSpy.clear();

            adapter.stopDiscovery();

            tryCompare(discoveringChangedSpy, "count", 1);
            compare(adapter.discovering, false);
        }
    }

    SignalSpy {
        id: managerDeviceRemovedSpy
        signalName: "deviceRemoved"
    }

    SignalSpy {
        id: adapterDeviceRemovedSpy
        signalName: "deviceRemoved"
    }

    SignalSpy {
        id: deviceDeviceRemovedSpy
        signalName: "deviceRemoved"
    }

    // za prefix to force test order - second last
    function test_za_RemoveDevice()
    {
        for (var i = 0; i < manager.adapters.length; ++i) {
            var adapter = manager.adapters[i];
            while (adapter.devices.length > 0) {
                var device = adapter.devices[0];

                managerDeviceRemovedSpy.clear();
                managerDeviceRemovedSpy.target = manager;
                adapterDeviceRemovedSpy.clear();
                adapterDeviceRemovedSpy.target = adapter;
                deviceDeviceRemovedSpy.clear();
                deviceDeviceRemovedSpy.target = device;

                adapter.removeDevice(device);

                tryCompare(managerDeviceRemovedSpy, "count", 1);
                tryCompare(adapterDeviceRemovedSpy, "count", 1);
                tryCompare(deviceDeviceRemovedSpy, "count", 1);
            }
        }
    }

    SignalSpy {
        id: managerAdapterRemovedSpy
        signalName: "adapterRemoved"
    }

    SignalSpy {
        id: adapterAdapterRemovedSpy
        signalName: "adapterRemoved"
    }

    // zb prefix to force test order - last
    function test_zb_AdapterRemoved()
    {
        while (manager.adapters.length > 0) {
            var adapter = manager.adapters[0];

            managerAdapterRemovedSpy.clear();
            managerAdapterRemovedSpy.target = manager;
            adapterAdapterRemovedSpy.clear();
            adapterAdapterRemovedSpy.target = adapter;

            var props = {
                Path: adapter.ubi,
                _toDBusObjectPath: [ "Path" ]
            }
            FakeBluez.runAction("devicemanager", "remove-adapter", props);

            tryCompare(managerAdapterRemovedSpy, "count", 1);
            tryCompare(adapterAdapterRemovedSpy, "count", 1);
        }
    }
}

