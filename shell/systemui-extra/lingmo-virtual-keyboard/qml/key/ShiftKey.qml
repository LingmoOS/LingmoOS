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
    id: shift
    label: "Shift"

    Binding {
        target: shift
        property: "switchKeyState"
        value: virtualKeyboard.shiftState
    }

    Component.onCompleted: {
        temporarySwitchKeyReleased.connect(updateShiftState)
        temporarySwitchKeyClicked.connect(virtualKeyboard.shiftClicked)
        temporarySwitchKeyLongPressed.connect(updateShiftState)
    }

    function updateShiftState(shiftState)
    {
        virtualKeyboard.shiftState = shiftState
    }

    state: virtualKeyboard.shiftState
    states: [
        State {
            name: "NORMAL"
            PropertyChanges {
                target: virtualKeyboard
                letterState : virtualKeyboard.capslockState == "NORMAL" ? "NORMAL" : "SHIFT"
                symbolState : "NORMAL"
            }
            PropertyChanges {
                target: keyBackground
                state: "NORMAL"
            }
        },
        State {
            name: "OPEN"
            PropertyChanges {
                target: virtualKeyboard
                letterState : virtualKeyboard.capslockState == "NORMAL" ? "SHIFT" : "NORMAL"
                symbolState : "SHIFT"
            }
            PropertyChanges {
                target: keyBackground
                state: "OPEN"
            }
        },
        State {
            name: "LONG_PRESSED"
            PropertyChanges {
                target: virtualKeyboard
                letterState : virtualKeyboard.capslockState == "NORMAL" ? "SHIFT" : "NORMAL"
                symbolState : "SHIFT"
            }
            PropertyChanges {
                target: keyBackground
                state: "PRESSED"
            }
        },
        State {
            name: "OPEN_LONG_PRESSED"
            PropertyChanges {
                target: virtualKeyboard
                letterState : virtualKeyboard.capslockState == "NORMAL" ? "SHIFT" : "NORMAL"
                symbolState : "SHIFT"
            }
            PropertyChanges {
                target: keyBackground
                state: "OPEN_PRESSED"
            }
        }
    ]
}
