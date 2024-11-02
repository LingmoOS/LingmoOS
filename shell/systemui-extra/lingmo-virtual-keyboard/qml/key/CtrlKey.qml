/*
* Copyright 2022 KylinSoft Co., Ltd.
*
* This program is free software: you can redistribute it and/or modify it under
* the terms of the GNU General Public License as published by the Free Software
* Foundation, either version 3 of the License, or (at your option) any later
* version.
*
* This program is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
* FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
* details.
*
* You should have received a copy of the GNU General Public License along with
* this program. If not, see <https://www.gnu.org/licenses/>.
*/

import QtQuick 2.0
import "../js/utils.js" as Utils

TemporarySwitchKey {
    id: ctrl
    label: "Ctrl"

    Binding {
        target: ctrl
        property: "switchKeyState"
        value: virtualKeyboard.ctrlState
    }

    Component.onCompleted: {
        temporarySwitchKeyReleased.connect(updateCtrlState)
        temporarySwitchKeyClicked.connect(virtualKeyboard.ctrlClicked)
    }

    function updateCtrlState(ctrlState)
    {
        virtualKeyboard.ctrlState = ctrlState
    }

    state: virtualKeyboard.ctrlState
    states: [
        State {
            name: "NORMAL"
            PropertyChanges {
                target: keyBackground
                state: "NORMAL"
            }
        },
        State {
            name: "OPEN"
            PropertyChanges {
                target: keyBackground
                state: "OPEN"
            }
        }
    ]

}
