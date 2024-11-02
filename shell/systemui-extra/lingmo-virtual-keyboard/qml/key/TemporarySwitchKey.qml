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
    signal temporarySwitchKeyReleased(string temporarySwitchKeyState)
    signal temporarySwitchKeyClicked()
    signal temporarySwitchKeyLongPressed(string temporarySwitchKeyState)

    multiPointTouchArea.onReleased: {
        longPressTimer_.stop()
        if(switchKeyState == "NORMAL") {
            switchKeyState = "OPEN"
        } else {
            switchKeyState = "NORMAL"
            var keysym = Utils.getKeysymByKeycode(keycode);
            var modifierKeyState = Utils.getModifierKeyStates()
            virtualKeyboard.processKeyEvent(keysym, keycode, modifierKeyState, true, Date())
            temporarySwitchKeyClicked()
        }
        temporarySwitchKeyReleased(switchKeyState)
    }

    multiPointTouchArea.onPressed: {
        longPressTimer_.start()

        var keysym = Utils.getKeysymByKeycode(keycode);
        var modifierKeyState = Utils.getModifierKeyStates()
        virtualKeyboard.processKeyEvent(keysym, keycode, modifierKeyState - Utils.getKeySym(label), false, Date())

    }

    Timer {
        id: longPressTimer_
        interval: 600
        onTriggered: {
            if (switchKeyState == "NORMAL") {
                temporarySwitchKeyLongPressed("LONG_PRESSED")
            } else if (switchKeyState == "OPEN") {
                temporarySwitchKeyLongPressed("OPEN_LONG_PRESSED")
            }
        }
    }
}
