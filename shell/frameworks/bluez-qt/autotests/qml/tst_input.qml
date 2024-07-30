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
    name: "Input"

    readonly property BluezQt.Manager manager: BluezQt.Manager

    property var input1props
    property var input2props
    property var input3props
    property var input4props
    property var device1props
    property var device2props
    property var device3props
    property var device4props

    function initTestCase()
    {
        FakeBluez.start();
        FakeBluez.runTest("bluez-standard");

        // Create adapter
        var adapter1path = "/org/bluez/hci0";
        var adapter1props = {
            Path: adapter1path,
            Address: "1C:E5:C3:BC:94:7E",
            Name: "TestAdapter",
            _toDBusObjectPath: [ "Path" ]
        }
        FakeBluez.runAction("devicemanager", "create-adapter", adapter1props);

        // Create devices
        input1props = {
            ReconnectMode: "none"
        }
        device1props = {
            Path: "/org/bluez/hci0/dev_40_79_6A_0C_39_75",
            Adapter: adapter1path,
            Address: "40:79:6A:0C:39:75",
            Name: "TestDevice",
            UUIDs: [ "00001124-0000-1000-8000-00805F9B34FB" ],
            Input: input1props,
            _toDBusObjectPath: [ "Path", "Adapter" ]
        }
        FakeBluez.runAction("devicemanager", "create-device", device1props);

        input2props = {
            ReconnectMode: "none"
        }
        device2props = {
            Path: "/org/bluez/hci0/dev_50_79_6A_0C_39_75",
            Adapter: adapter1path,
            Address: "50:79:6A:0C:39:75",
            Name: "TestDevice2",
            UUIDs: [ "00001124-0000-1000-8000-00805F9B34FB" ],
            Input: input2props,
            _toDBusObjectPath: [ "Path", "Adapter" ]
        }
        FakeBluez.runAction("devicemanager", "create-device", device2props);

        input3props = {
            ReconnectMode: "none"
        }
        device3props = {
            Path: "/org/bluez/hci0/dev_60_79_6A_0C_39_75",
            Adapter: adapter1path,
            Address: "60:79:6A:0C:39:75",
            Name: "TestDevice2",
            UUIDs: [ "00001124-0000-1000-8000-00805F9B34FB" ],
            Input: input3props,
            _toDBusObjectPath: [ "Path", "Adapter" ]
        }
        FakeBluez.runAction("devicemanager", "create-device", device3props);

        input4props = {
            ReconnectMode: "none"
        }
        device4props = {
            Path: "/org/bluez/hci0/dev_70_79_6A_0C_39_75",
            Adapter: adapter1path,
            Address: "70:79:6A:0C:39:75",
            Name: "TestDevice2",
            UUIDs: [ "00001124-0000-1000-8000-00805F9B34FB" ],
            Input: input4props,
            _toDBusObjectPath: [ "Path", "Adapter" ]
        }
        FakeBluez.runAction("devicemanager", "create-device", device4props);

        tryCompare(manager, "operational", true);
        compare(manager.adapters.length, 1, "adapters-length");
        compare(manager.devices.length, 4, "devices-length");
    }

    function cleanupTestCase()
    {
        FakeBluez.stop();
    }

    function compareProperties(input, props)
    {
        compare(modeString(input), props.ReconnectMode, "reconnectMode");
    }

    function test_getProperties()
    {
        const device1 = manager.deviceForUbi(device1props.Path);
        const device2 = manager.deviceForUbi(device2props.Path);
        const device3 = manager.deviceForUbi(device3props.Path);
        const device4 = manager.deviceForUbi(device4props.Path);

        compareProperties(device1.input, input1props);
        compareProperties(device2.input, input2props);
        compareProperties(device3.input, input3props);
        compareProperties(device4.input, input4props);
    }

    function modeString(input)
    {
        switch (input.reconnectMode) {
        case BluezQt.Input.NoReconnect:
            return "none";
        case BluezQt.Input.HostReconnect:
            return "host";
        case BluezQt.Input.DeviceReconnect:
            return "device";
        default:
            return "any";
        }
    }
}
