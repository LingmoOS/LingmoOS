/*
    SPDX-FileCopyrightText: 2012 Gregor Taetzner <gregor@freenet.de>
    SPDX-FileCopyrightText: 2020 Ivan Čukić <ivan.cukic at kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import org.kde.lingmo.plasmoid
import org.kde.lingmo.core as LingmoCore
import org.kde.lingmo.components as LingmoComponents3
import org.kde.draganddrop as DND

import org.kde.lingmo.activityswitcher as ActivitySwitcher
import org.kde.lingmoui as LingmoUI
import org.kde.activities as Activities

PlasmoidItem {
    id: root

    width: LingmoUI.Units.iconSizes.large
    height: LingmoUI.Units.iconSizes.large

    Layout.maximumWidth: Infinity
    Layout.maximumHeight: Infinity

    Layout.preferredWidth : height + (root.showActivityName ? name.implicitWidth + 2 * LingmoUI.Units.smallSpacing : 0)

    Layout.minimumWidth: 0
    Layout.minimumHeight: 0

    readonly property bool inVertical: Plasmoid.formFactor === LingmoCore.Types.Vertical
    readonly property bool showActivityName: Plasmoid.configuration.showActivityName
    readonly property bool showActivityIcon: Plasmoid.configuration.showActivityIcon

    Plasmoid.onActivated: ActivitySwitcher.Backend.toggleActivityManager()
    preferredRepresentation: fullRepresentation

    DND.DropArea {
        id: dropArea
        anchors.fill: parent

        onDragEnter: ActivitySwitcher.Backend.setDropMode(true)
        onDragLeave: ActivitySwitcher.Backend.setDropMode(false)

        Activities.ActivityInfo {
            id: currentActivity
            activityId: ":current"
        }

        MouseArea {
            anchors.fill: parent

            activeFocusOnTab: true

            Keys.onPressed: {
                switch (event.key) {
                case Qt.Key_Space:
                case Qt.Key_Enter:
                case Qt.Key_Return:
                case Qt.Key_Select:
                    Plasmoid.activated();
                    break;
                }
            }
            Accessible.name: name.text ? i18nc("@info:tooltip", "Current activity is %1", name.text) : ""
            Accessible.description: tooltip.subText
            Accessible.role: Accessible.Button

            onClicked: Plasmoid.activated()
        }

        LingmoCore.ToolTipArea {
            id: tooltip
            anchors.fill: parent
            mainText: i18n("Show Activity Manager")
            subText: i18n("Click to show the activity manager")
        }

        LingmoUI.Icon {
            id: icon

            anchors.verticalCenter: parent.verticalCenter

            height: Math.min(parent.height, parent.width)
            width: height

            source: !root.showActivityIcon ? "activities" :
                    currentActivity.icon === "" ? "activities" :
                    currentActivity.icon
        }

        LingmoComponents3.Label {
            id: name

            anchors {
                left: icon.right
                leftMargin: LingmoUI.Units.smallSpacing
                rightMargin: LingmoUI.Units.smallSpacing
            }
            height: parent.height
            width: parent.width - icon.width
            visible: root.showActivityName && !root.inVertical
            elide: Text.ElideRight

            verticalAlignment: Text.AlignVCenter

            text: currentActivity.name
            textFormat: Text.PlainText
        }
    }
}
