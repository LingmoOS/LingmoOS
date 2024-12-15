// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.3

import org.lingmo.oceanui 1.0
import org.lingmo.dtk 1.0 as D

OceanUIObject {
    OceanUIObject {
        name: "general"
        parentName: "power"
        displayName: qsTr("General")
        description: qsTr("Power plans, power saving settings, wakeup settings, shutdown settings")
        icon: "general"
        weight: 10
        page: OceanUIRightView {
            spacing: 5
        }
        GeneralPage {}
    }
    OceanUIObject {
        name: "onPower"
        parentName: "power"
        displayName: qsTr("Plugged In")
        description: qsTr("Screen and suspend")
        icon: "plugged_in"
        weight: 100
        visible: oceanuiData.platformName() !== "wayland"
        page: OceanUIRightView {
            spacing: 5
        }
        PowerPage {}
    }
    OceanUIObject {
        name: "onBattery"
        parentName: "power"
        displayName: qsTr("On Battery")
        description: qsTr("screen and suspend, low battery, battery management")
        icon: "on_battery"
        weight: 200
        visible: oceanuiData.model.haveBettary
        page: OceanUIRightView {
            spacing: 5
        }
        BatteryPage {}
    }
}
