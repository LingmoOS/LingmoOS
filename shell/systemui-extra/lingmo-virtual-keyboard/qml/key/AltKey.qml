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
    id: alt
    label: "Alt"

    Binding {
        target: alt
        property: "switchKeyState"
        value: virtualKeyboard.altState
    }

    Component.onCompleted: {
        temporarySwitchKeyReleased.connect(updateAltState)
        temporarySwitchKeyClicked.connect(virtualKeyboard.altClicked)
    }

    function updateAltState(altState)
    {
        virtualKeyboard.altState = altState
    }

    state: virtualKeyboard.altState
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
