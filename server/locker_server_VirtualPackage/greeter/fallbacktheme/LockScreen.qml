/*
SPDX-FileCopyrightText: 2011 Martin Gräßlin <mgraesslin@kde.org>

SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15

import org.kde.kquickcontrolsaddons 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.private.sessions 2.0

Item {
    id: lockScreen

    property alias capsLockOn: unlockUI.capsLockOn
    property bool locked: false

    signal unlockRequested()

    // if there's no image, have a near black background
    Rectangle {
        width: parent.width
        height: parent.height
        color: "#111"
    }

    SessionsModel {
        id: sessionsModel
    }

    Image {
        id: background

        anchors.fill: parent
        source: "file:" + theme.wallpaperPathForSize(parent.width, parent.height)
        smooth: true
    }

    PlasmaCore.FrameSvgItem {
        id: dialog

        visible: lockScreen.locked
        anchors.centerIn: parent
        imagePath: "widgets/background"
        width: mainStack.currentPage.implicitWidth + margins.left + margins.right
        height: mainStack.currentPage.implicitHeight + margins.top + margins.bottom

        Behavior on height {
            enabled: mainStack.currentPage != null
            NumberAnimation {
                duration: 250
            }
        }
        Behavior on width {
            enabled: mainStack.currentPage != null
            NumberAnimation {
                duration: 250
            }
        }
        PlasmaComponents.PageStack {
            id: mainStack

            clip: true
            anchors {
                fill: parent
                leftMargin: dialog.margins.left
                topMargin: dialog.margins.top
                rightMargin: dialog.margins.right
                bottomMargin: dialog.margins.bottom
            }
            initialPage: unlockUI
        }
    }

    Greeter {
        id: unlockUI

        switchUserEnabled: sessionsModel.canSwitchUser

        Connections {
            function onAccepted() {
                lockScreen.unlockRequested();
            }
            function onSwitchUserClicked() {
                mainStack.push(userSessionsUIComponent);
                mainStack.currentPage.forceActiveFocus();
            }
        }
    }

    function returnToLogin() {
        mainStack.pop();
        unlockUI.resetFocus();
    }

    Component {
        id: userSessionsUIComponent

        SessionSwitching {
            id: userSessionsUI

            visible: false

            Connections {
                function onSwitchingCanceled() {
                    returnToLogin();
                }
                function onSessionActivated() {
                    returnToLogin();
                }
                function onNewSessionStarted() {
                    returnToLogin();
                }
            }
        }
    }
}
