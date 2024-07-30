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
import org.kde.kirigami as Kirigami
import org.kde.plasma.components as PlasmaComponents3
import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.extras as PlasmaExtras
import org.kde.plasma.plasmoid

PlasmaExtras.PlasmoidHeading {
    id: root

    required property PlasmoidItem plasmoidItem
    required property PlasmaCore.Action addDeviceAction
    required property PlasmaCore.Action toggleBluetoothAction

    readonly property alias onSwitch: onSwitch

    leftPadding: mirrored ? 0 : Kirigami.Units.smallSpacing
    rightPadding: mirrored ? Kirigami.Units.smallSpacing : 0

    contentItem: RowLayout {
        spacing: Kirigami.Units.smallSpacing

        PlasmaComponents3.Switch {
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

        PlasmaComponents3.ToolButton {
            id: addDeviceButton

            readonly property PlasmaCore.Action qAction: root.addDeviceAction

            visible: !(Plasmoid.containmentDisplayHints & PlasmaCore.Types.ContainmentDrawsPlasmoidHeading)
            enabled: qAction.visible

            icon.name: "list-add-symbolic"

            onClicked: qAction.trigger()

            PlasmaComponents3.ToolTip {
                text: addDeviceButton.qAction.text
            }
            Accessible.name: qAction.text
        }

        PlasmaComponents3.ToolButton {
            id: openSettingsButton

            readonly property PlasmaCore.Action qAction: Plasmoid.internalAction("configure")

            visible: !(Plasmoid.containmentDisplayHints & PlasmaCore.Types.ContainmentDrawsPlasmoidHeading)
            icon.name: "configure-symbolic"
            onClicked: qAction.trigger()

            PlasmaComponents3.ToolTip {
                text: openSettingsButton.qAction.text
            }
            Accessible.name: qAction.text
        }
    }
}
