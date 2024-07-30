// SPDX-FileCopyrightText: 2021 Anupam Basak <anupam.basak27@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2021 Aditya Mehra <aix.m@outlook.com>
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtGraphicalEffects 1.0

import org.kde.kirigami 2.9 as Kirigami

Button {
    id: nextButtonComponent
    property bool accepted: false

    signal nextClicked()

    icon.name: "go-next"
    text: "Next"

    QtObject {
        id: privateProps

        property bool showSpinner: false
    }

    Keys.onReturnPressed: {
        clicked()
    }

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

    Kirigami.Icon {
        width: Kirigami.Units.iconSizes.smallMedium
        height: Kirigami.Units.iconSizes.smallMedium
        // running: privateProps.showSpinner

        states: [
            State {
                when: privateProps.showSpinner;
                PropertyChanges {
                    target: nextButtonComponent
                    enabled: false
                }
            },
            State {
                when: !privateProps.showSpinner;
                PropertyChanges {
                    target: nextButtonComponent
                    enabled: true
                }
            }
        ]
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
