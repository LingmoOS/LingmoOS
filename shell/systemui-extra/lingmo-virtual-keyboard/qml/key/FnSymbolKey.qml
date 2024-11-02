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

SymbolKey {
    height: virtualKeyboard.firstRowKeyHeight

    property string fnValue
    property int fnKeycode
    property int numberKeycode
    
    keycode: virtualKeyboard.fnSymbolState == "FN" ? fnKeycode : numberKeycode

    fontSize: virtualKeyboard.fnSymbolKeyFontSize

    state: virtualKeyboard.fnSymbolState

    states: [
        State {
            name: "FN"
            PropertyChanges {
                target: keyLabel
                text: fnValue
            }
            PropertyChanges {
                target: shiftLabel
                text: ""
            }
        }
    ]
}
