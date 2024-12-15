// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
import org.lingmo.oceanui 1.0

OceanUIObject {
    id: root
    name: "touchscreen"
    parentName: "device"
    displayName: qsTr("Touchscreen")
    description: qsTr("Configuring Touchscreen")
    icon: "oceanui_nav_touchscreen"
    visible: false
    weight: 60
    OceanUIDBusInterface {
        property var touchscreensV2
        service: "org.lingmo.ocean.Display1"
        path: "/org/lingmo/ocean/Display1"
        inter: "org.lingmo.ocean.Display1"
        connection: OceanUIDBusInterface.SessionBus
        onTouchscreensV2Changed: {
            root.visible = touchscreensV2.length !== 0
        }
    }
}
