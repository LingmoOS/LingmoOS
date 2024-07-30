/*
SPDX-FileCopyrightText: 2011 Martin Gräßlin <mgraesslin@kde.org>
SPDX-FileCopyrightText: 2023 Nate Graham <nate@kde.org>

SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Controls as QQC2

import org.kde.kirigami 2.20 as Kirigami
import org.kde.plasma.core as PlasmaCore
import org.kde.kquickcontrolsaddons 2.0
import org.kde.ksvg 1.0 as KSvg
import org.kde.plasma.private.sessions 2.0

Item {
    id: lockScreen

    property alias capsLockOn: unlockUI.capsLockOn
    property bool locked: false

    signal unlockRequested()

    // if there's no image, have a pure black background
    Rectangle {
        width: parent.width
        height: parent.height
        color: "black"
    }

    SessionManagement {
        id: sessionManagment
    }

    Image {
        anchors.fill: parent
        source: "file:" + PlasmaCore.Theme.wallpaperPathForSize(parent.width, parent.height)
        smooth: true
    }

    KSvg.FrameSvgItem {
        id: dialog

        visible: lockScreen.locked
        anchors.centerIn: parent
        width: mainStack.currentItem.implicitWidth + margins.left + margins.right
        height: mainStack.currentItem.implicitHeight + margins.top + margins.bottom
        imagePath: "widgets/background"

        Behavior on height {
            enabled: mainStack.currentItem != null
            NumberAnimation {
                duration: Kirigami.Units.longDuration
            }
        }
        Behavior on width {
            enabled: mainStack.currentItem != null
            NumberAnimation {
                duration: Kirigami.Units.longDuration
            }
        }

        QQC2.StackView {
            id: mainStack

            clip: true
            anchors {
                fill: parent
                leftMargin: dialog.margins.left
                topMargin: dialog.margins.top
                rightMargin: dialog.margins.right
                bottomMargin: dialog.margins.bottom
            }
            initialItem: unlockUI
        }
    }

    Greeter {
        id: unlockUI

        switchUserEnabled: sessionManagment.canSwitchUser

        visible: opacity > 0
        opacity: mainStack.currentItem == unlockUI
        Behavior on opacity {
            NumberAnimation {
                duration: Kirigami.Units.longDuration
            }
        }

        Connections {
            function onAccepted() {
                lockScreen.unlockRequested();
            }
            function onSwitchUserClicked() {
                sessionManagment.switchUser();
            }
        }
    }

    function returnToLogin() {
        mainStack.pop();
        unlockUI.resetFocus();
    }
}
