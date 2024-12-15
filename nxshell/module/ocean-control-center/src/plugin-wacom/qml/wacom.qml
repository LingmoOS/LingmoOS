// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
import org.lingmo.oceanui 1.0

OceanUIObject {
    id: root
    name: "wacom"
    parentName: "device"
    displayName: qsTr("wacom")
    description: qsTr("Configuring wacom")
    icon: "oceanui_nav_wacom"
    visible: false
    weight: 50
    OceanUIDBusInterface {
        property var exist
        service: "org.lingmo.ocean.InputDevices1"
        path: "/org/lingmo/ocean/InputDevice1/Wacom"
        inter: "org.lingmo.ocean.InputDevice1.Wacom"
        connection: OceanUIDBusInterface.SessionBus
        onExistChanged: {
            root.visible = exist
        }
    }
}
