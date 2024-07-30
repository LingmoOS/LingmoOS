/*
    SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>
    SPDX-FileCopyrightText: 2015 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts

import org.kde.lingmoui as LingmoUI
import org.kde.lingmo.core as LingmoCore
import org.kde.lingmo.plasmoid

LingmoCore.ToolTipArea {
    id: tooltip

    readonly property int arrowAnimationDuration: LingmoUI.Units.shortDuration
    property bool vertical: Plasmoid.formFactor === LingmoCore.Types.Vertical
    property int iconSize: LingmoUI.Units.iconSizes.smallMedium
    implicitWidth: iconSize
    implicitHeight: iconSize
    activeFocusOnTab: true

    Accessible.name: subText
    Accessible.description: i18n("Show all the items in the system tray in a popup")
    Accessible.role: Accessible.Button
    Accessible.onPressAction: systemTrayState.expanded = !systemTrayState.expanded

    Keys.onPressed: event => {
        switch (event.key) {
        case Qt.Key_Space:
        case Qt.Key_Enter:
        case Qt.Key_Return:
        case Qt.Key_Select:
            systemTrayState.expanded = !systemTrayState.expanded;
        }
    }

    subText: systemTrayState.expanded ? i18n("Close popup") : i18n("Show hidden icons")

    property bool wasExpanded

    TapHandler {
        onPressedChanged: {
            if (pressed) {
                tooltip.wasExpanded = systemTrayState.expanded;
            }
        }
        onTapped: (eventPoint, button) => {
            systemTrayState.expanded = !tooltip.wasExpanded;
            expandedRepresentation.hiddenLayout.currentIndex = -1;
        }
    }

    LingmoUI.Icon {
        anchors.fill: parent

        rotation: systemTrayState.expanded ? 180 : 0
        Behavior on rotation {
            RotationAnimation {
                duration: tooltip.arrowAnimationDuration
            }
        }
        opacity: systemTrayState.expanded ? 0 : 1
        Behavior on opacity {
            NumberAnimation {
                duration: tooltip.arrowAnimationDuration
            }
        }

        source: {
            if (Plasmoid.location === LingmoCore.Types.TopEdge) {
                return "arrow-down";
            } else if (Plasmoid.location === LingmoCore.Types.LeftEdge) {
                return "arrow-right";
            } else if (Plasmoid.location === LingmoCore.Types.RightEdge) {
                return "arrow-left";
            } else {
                return "arrow-up";
            }
        }
    }

    LingmoUI.Icon {
        anchors.fill: parent

        rotation: systemTrayState.expanded ? 0 : -180
        Behavior on rotation {
            RotationAnimation {
                duration: tooltip.arrowAnimationDuration
            }
        }
        opacity: systemTrayState.expanded ? 1 : 0
        Behavior on opacity {
            NumberAnimation {
                duration: tooltip.arrowAnimationDuration
            }
        }

        source: {
            if (Plasmoid.location === LingmoCore.Types.TopEdge) {
                return "arrow-up";
            } else if (Plasmoid.location === LingmoCore.Types.LeftEdge) {
                return "arrow-left";
            } else if (Plasmoid.location === LingmoCore.Types.RightEdge) {
                return "arrow-right";
            } else {
                return "arrow-down";
            }
        }
    }
}
