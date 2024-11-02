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

SwitchKey {
    label: "Caps"

    switchKeyState: virtualKeyboard.capslockState
    multiPointTouchArea.onReleased: {
        if(virtualKeyboard.capslockState == "NORMAL") {
            virtualKeyboard.capslockState = "OPEN"
        } else {
            virtualKeyboard.capslockState = "NORMAL"
        }

        var keysym = Utils.getKeysymByKeyLabel(label)
        var modifierKeyState = Utils.getModifierKeyStates()
        virtualKeyboard.processKeyEvent(keysym, keycode, modifierKeyState, false, Date())
        virtualKeyboard.processKeyEvent(keysym, keycode, modifierKeyState, true, Date())
    }

    state: virtualKeyboard.capslockState
    states: [
        State {
            name: "NORMAL"
            PropertyChanges {
                target: virtualKeyboard
                letterState : virtualKeyboard.shiftState == "NORMAL" ? "NORMAL" : "SHIFT"
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
                letterState : virtualKeyboard.shiftState == "NORMAL" ? "SHIFT" : "NORMAL"
            }
            PropertyChanges {
                target: keyBackground
                state: "OPEN"
            }
        }
    ]
}
