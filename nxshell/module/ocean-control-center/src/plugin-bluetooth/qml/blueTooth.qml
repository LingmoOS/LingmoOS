// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick 2.15
import QtQuick.Controls 2.0

import org.lingmo.oceanui 1.0

OceanUIObject {
    id: root
    name: "bluetooth"
    parentName: "device"
    displayName: qsTr("bluetooth")
    description: qsTr("Bluetooth settings, devices")
    visible: false
    icon: "bluetoothNomal"
    weight: 70

    OceanUIDBusInterface {
        id: bluetoothDbus
        service: "org.lingmo.ocean.Bluetooth1"
        path: "/org/lingmo/ocean/Bluetooth1"
        inter: "org.lingmo.ocean.Bluetooth1"
        connection: OceanUIDBusInterface.SessionBus

        function errorSlot(adapters) {
            console.log("bluetooth GetAdapters errorSlot : ", adapters)
        }

        function getAdaptersSlot(adapters) {
            console.log(" bluetooth onGetAdapters", adapters)
            if (adapters === "[]") {
                root.visible = false
            } else {
                root.visible = true
            }
        }

        function onAdapterAoceand(adapters) {
            root.visible = true
        }
        function onAdapterRemoved(adapters) {
            bluetoothDbus.callWithCallback("GetAdapters", [], getAdaptersSlot, errorSlot)
        }

        Component.onCompleted: {
            callWithCallback("GetAdapters", [], getAdaptersSlot, errorSlot)
        }
    }
}

