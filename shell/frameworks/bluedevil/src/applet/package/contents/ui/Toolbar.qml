/*
    SPDX-FileCopyrightText: 2013-2014 Jan Grulich <jgrulich@redhat.com>
    SPDX-FileCopyrightText: 2014-2015 David Rosca <nowrep@gmail.com>
    SPDX-FileCopyrightText: 2024 ivan tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

import org.kde.bluezqt as BluezQt
import org.kde.lingmoui as LingmoUI
import org.kde.lingmo.components as LingmoComponents3
import org.kde.lingmo.core as LingmoCore
import org.kde.lingmo.extras as LingmoExtras
import org.kde.lingmo.plasmoid

LingmoExtras.PlasmoidHeading {
    id: root

    required property PlasmoidItem plasmoidItem
    required property LingmoCore.Action addDeviceAction
    required property LingmoCore.Action toggleBluetoothAction

    readonly property alias onSwitch: onSwitch

    leftPadding: mirrored ? 0 : LingmoUI.Units.smallSpacing
    rightPadding: mirrored ? LingmoUI.Units.smallSpacing : 0

    contentItem: RowLayout {
        spacing: LingmoUI.Units.smallSpacing

        LingmoComponents3.Switch {
            id: onSwitch
            text: root.toggleBluetoothAction.text
            icon.name: root.toggleBluetoothAction.icon.name
            checked: root.toggleBluetoothAction.checked
            enabled: root.toggleBluetoothAction.visible
            focus: root.plasmoidItem.expanded
            onToggled: root.toggleBluetoothAction.trigger()
        }

        Item {
            Layout.fillWidth: true
        }

        LingmoComponents3.ToolButton {
            id: addDeviceButton

            readonly property LingmoCore.Action qAction: root.addDeviceAction

            visible: !(Plasmoid.containmentDisplayHints & LingmoCore.Types.ContainmentDrawsPlasmoidHeading)
            enabled: qAction.visible

            icon.name: "list-add-symbolic"

            onClicked: qAction.trigger()

            LingmoComponents3.ToolTip {
                text: addDeviceButton.qAction.text
            }
            Accessible.name: qAction.text
        }

        LingmoComponents3.ToolButton {
            id: openSettingsButton

            readonly property LingmoCore.Action qAction: Plasmoid.internalAction("configure")

            visible: !(Plasmoid.containmentDisplayHints & LingmoCore.Types.ContainmentDrawsPlasmoidHeading)
            icon.name: "configure-symbolic"
            onClicked: qAction.trigger()

            LingmoComponents3.ToolTip {
                text: openSettingsButton.qAction.text
            }
            Accessible.name: qAction.text
        }
    }
}
