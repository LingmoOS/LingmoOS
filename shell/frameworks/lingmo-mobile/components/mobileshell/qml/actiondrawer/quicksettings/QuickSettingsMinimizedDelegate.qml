/*
 *   SPDX-FileCopyrightText: 2021 Devin Lin <devin@kde.org>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick 2.1
import QtQuick.Layouts 1.1

import org.kde.lingmoui 2.12 as LingmoUI

import org.kde.lingmo.private.mobileshell as MobileShell
import org.kde.lingmo.core as LingmoCore
import org.kde.lingmo.private.nanoshell 2.0 as NanoShell
import org.kde.lingmo.private.mobileshell.shellsettingsplugin as ShellSettings
import org.kde.lingmo.components 3.0 as LingmoComponents

QuickSettingsDelegate {
    id: root

    iconItem: icon

    // scale animation on press
    zoomScale: (ShellSettings.Settings.animationsEnabled && mouseArea.pressed) ? 0.9 : 1

    background: Item {
        // very simple shadow for performance
        Rectangle {
            anchors.top: parent.top
            anchors.topMargin: 1
            anchors.left: parent.left
            anchors.right: parent.right
            height: parent.height

            radius: LingmoUI.Units.cornerRadius
            color: Qt.rgba(0, 0, 0, 0.075)
        }

        // background
        Rectangle {
            anchors.fill: parent
            radius: LingmoUI.Units.cornerRadius
            border.color: root.enabled ? root.enabledButtonBorderColor : root.disabledButtonBorderColor
            color: {
                if (root.enabled) {
                    return mouseArea.pressed ? root.enabledButtonPressedColor : root.enabledButtonColor
                } else {
                    return mouseArea.pressed ? root.disabledButtonPressedColor : root.disabledButtonColor
                }
            }
        }
    }

    MobileShell.HapticsEffect {
        id: haptics
    }

    contentItem: MouseArea {
        id: mouseArea

        onPressed: haptics.buttonVibrate();
        onClicked: root.delegateClick()
        onPressAndHold: {
            haptics.buttonVibrate();
            root.delegatePressAndHold();
        }

        cursorShape: Qt.PointingHandCursor

        LingmoUI.Icon {
            id: icon
            anchors.centerIn: parent
            implicitWidth: LingmoUI.Units.iconSizes.smallMedium
            implicitHeight: width
            source: root.icon
        }
    }
}

