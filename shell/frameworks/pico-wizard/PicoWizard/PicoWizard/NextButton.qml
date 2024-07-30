// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2021 Anupam Basak <anupam.basak27@gmail.com>

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtGraphicalEffects 1.0

import org.kde.lingmoui 2.9 as LingmoUI

RoundButton {
    property bool accepted: false

    signal nextClicked()

    QtObject {
        id: privateProps

        property bool showSpinner: false
    }

    width: LingmoUI.Units.iconSizes.huge
    height: LingmoUI.Units.iconSizes.huge

    flat: true
    onClicked: {
        if (!accepted) {
            nextClicked()

            if (!accepted) {
                next()
            } else {
                privateProps.showSpinner = true
            }
        }
    }

    background: Rectangle {
        color: enabled ? LingmoUI.Theme.highlightColor : LingmoUI.Theme.disabledTextColor
        radius: parent.width
    }

    LingmoUI.Icon {
        id: nextIcon
        width: LingmoUI.Units.iconSizes.smallMedium
        height: LingmoUI.Units.iconSizes.smallMedium
        isMask: true

        anchors.centerIn: parent
        color: LingmoUI.Theme.highlightedTextColor
        source: "go-next"

        states: [
            State {
                when: !privateProps.showSpinner;
                PropertyChanges {
                    target: nextIcon
                    opacity: 1.0
                }
            },
            State {
                when: privateProps.showSpinner;
                PropertyChanges {
                    target: nextIcon
                    opacity: 0.0
                }
            }
        ]
        transitions: Transition {
            NumberAnimation {
                property: "opacity"
                duration: LingmoUI.Units.longDuration
                easing.type: Easing.InOutQuad
            }
        }

    }

    BusyIndicator {
        id: spinnerIcon
        width: LingmoUI.Units.iconSizes.smallMedium
        height: LingmoUI.Units.iconSizes.smallMedium

        anchors.centerIn: parent
        running: privateProps.showSpinner
        visible: privateProps.showSpinner
    }

    Timer {
        id: resetSpinnerTimer
        repeat: false
        interval: 500
        onTriggered: {
            accepted = false
            privateProps.showSpinner = false
        }
    }

    function next() {
        moduleLoader.nextModule()
        resetSpinnerTimer.start()
    }
    function cancel() {
        resetSpinnerTimer.start()
    }
}
