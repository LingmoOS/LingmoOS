// Copyright (C) 2023 justforlxz <justforlxz@gmail.com>.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import TreeLand.Greeter

FocusScope {
    id: root
    clip: true

    // prevent event passing through greeter
    MouseArea {
        anchors.fill: parent
        enabled: true
    }

    Image {
        id: background
        source: "file:///usr/share/wallpapers/lingmo/desktop.jpg"
        fillMode: Image.PreserveAspectCrop
        anchors.fill: parent
        ScaleAnimator on scale {
            from: 1
            to: 1.1
            duration: 400
        }
    }

    ParallelAnimation {
        id: backgroundAni
        ScaleAnimator {
            target: background
            from: 1.1
            to: 1
            duration: 400
        }
        PropertyAnimation {
            target: background
            property: "opacity"
            duration: 600
            from: 1
            to: 0
            easing.type: Easing.InQuad
        }
    }

    Center {
        id: center
        anchors.fill: parent
        anchors.leftMargin: 50
        anchors.topMargin: 50
        anchors.rightMargin: 50
        anchors.bottomMargin: 50

        focus: true
    }


    function checkUser(userName) {
        let user = GreeterModel.userModel.get(GreeterModel.currentUser)
        console.log("last activate user:",user.name,"current user:",userName)
        return user.name == userName
    }

    Connections {
        target: GreeterModel.proxy
        function onLoginSucceeded(userName) {
            if (!checkUser(userName)) {
                return
            }

            center.loginGroup.userAuthSuccessed()
            center.loginGroup.updateHintMsg(center.loginGroup.normalHint)
            GreeterModel.emitAnimationPlayed()
        }
    }

    Connections {
        target: GreeterModel.proxy
        function onLoginFailed(userName) {
            if (!checkUser(userName)) {
                return
            }

            center.loginGroup.userAuthFailed()
            center.loginGroup.updateHintMsg(qsTr("Password is incorrect."))
        }
    }

    Connections {
        target: GreeterModel
        function onAnimationPlayed() {
            backgroundAni.start()
        }
        function onAnimationPlayFinished() {
            background.scale = 1.1
            background.opacity = 1
        }
    }
}
