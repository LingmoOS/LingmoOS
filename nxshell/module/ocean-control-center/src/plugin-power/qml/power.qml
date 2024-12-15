// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
import org.lingmo.oceanui 1.0

OceanUIObject {
    id: power
    name: "power"
    parentName: "root"
    displayName: qsTr("power")
    description: qsTr("Power saving settings, screen and suspend")
    icon: "power"
    weight: 50

    visible: false
    OceanUIDBusInterface {
        property var onBattery
        service: "org.lingmo.ocean.Power1"
        path: "/org/lingmo/ocean/Power1"
        inter: "org.lingmo.ocean.Power1"
        connection: OceanUIDBusInterface.SessionBus
        onOnBatteryChanged: power.visible = true
    }
}
