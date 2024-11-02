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

function getKeySym(keyName) {
    var keySymList = {
        "shift": 1,
        "caps": 2,
        "ctrl": 4,
        "alt": 8,
        "win": 64
    }
    return keySymList[keyName.toLowerCase()]
}

function getModifierKeyStates() {
    var result = 0
    result += shiftState == "NORMAL" ? 0 : getKeySym("shift")
    result += capslockState == "NORMAL" ? 0 : getKeySym("caps")
    result += ctrlState == "NORMAL" ? 0 : getKeySym("ctrl")
    result += altState == "NORMAL" ? 0 : getKeySym("alt")
    result += winState == "NORMAL" ? 0 : getKeySym("win")
    return result
}

function getKeysymByKeyLabel(keyLabel) {
    var keysymMap = {
        "esc": 0xff1b,
        "tab": 0xff09,
        "caps": 0xffe5,
        "win": 0xffeb,
        "backspace": 0xff08,
        "enter": 0xff0d,
        "left": 0xff51,
        "up": 0xff52,
        "right": 0xff53,
        "down": 0xff54,
        "del": 0xffff,
        "space": 0x0020,
        "f1": 0xffbe,
        "f2": 0xffbf,
        "f3": 0xffc0,
        "f4": 0xffc1,
        "f5": 0xffc2,
        "f6": 0xffc3,
        "f7": 0xffc4,
        "f8": 0xffc5,
        "f9": 0xffc6,
        "f10": 0xffc7,
        "f11": 0xffc8,
        "f12": 0xffc9
    }

    if (keysymMap.hasOwnProperty(keyLabel.toLowerCase())) {
        return keysymMap[keyLabel.toLowerCase()]
    }

    return keyLabel[0].charCodeAt(0)
}

function getKeysymByKeycode(keycode) {
    var keysymMap = {
        "50": 0xffe1,   // Shift_L
        "62": 0xffe2,   // Shift_R
        "37": 0xffe3,   // Ctrl_L
        "105": 0xffe4,  // Ctrl_R
        "64": 0xffe9,   // Alt_L
        "108": 0xffea   // Alt_R
    }

    if (keysymMap.hasOwnProperty(keycode)) {
        return keysymMap[keycode]
    }

    return 0;
}





