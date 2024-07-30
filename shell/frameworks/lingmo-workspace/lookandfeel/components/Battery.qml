/*
    SPDX-FileCopyrightText: 2016 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.15

import org.kde.lingmo.components 3.0 as LingmoComponents3
import org.kde.lingmo.workspace.components 2.0 as PW
import org.kde.lingmoui 2.20 as LingmoUI

import org.kde.lingmo.private.battery

RowLayout {
    id: root

    property real fontSize: LingmoUI.Theme.defaultFont.pointSize

    BatteryControlModel {
        id: batteryControl
    }

    spacing: LingmoUI.Units.smallSpacing
    visible: batteryControl.hasInternalBatteries

    PW.BatteryIcon {
        pluggedIn: batteryControl.pluggedIn
        hasBattery: batteryControl.hasCumulative
        percent: batteryControl.percent

        Layout.preferredHeight: Math.max(LingmoUI.Units.iconSizes.medium, batteryLabel.implicitHeight)
        Layout.preferredWidth: Layout.preferredHeight
        Layout.alignment: Qt.AlignVCenter
    }

    LingmoComponents3.Label {
        id: batteryLabel
        font.pointSize: root.fontSize
        text: i18nd("lingmo_lookandfeel_org.kde.lookandfeel", "%1%", batteryControl.percent)
        textFormat: Text.PlainText
        Accessible.name: i18nd("lingmo_lookandfeel_org.kde.lookandfeel", "Battery at %1%", batteryControl.percent)
        Layout.alignment: Qt.AlignVCenter
    }
}
