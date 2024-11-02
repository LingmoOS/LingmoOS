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
import "key/"
import "../qml/js/keycode.js" as Keycode

Column {
    width: parent.width
    spacing: virtualKeyboard.keySpacing
    visible: virtualKeyboard.layout == "classic" ? true : false

    Row {
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: virtualKeyboard.keySpacing
        ActionKey{ label: "Esc"; height: virtualKeyboard.keyHeight * 3/4; keycode: Keycode.KEY_ESC}
        SymbolKey{ label: "`"; shiftedText: "~"; height: virtualKeyboard.firstRowKeyHeight; keycode: Keycode.KEY_GRAVE }
        FnSymbolKey{ label: "1"; shiftedText: "!"; fnValue:"F1"; fnKeycode: Keycode.KEY_F1; numberKeycode: Keycode.KEY_1 }
        FnSymbolKey{ label: "2"; shiftedText: "@"; fnValue:"F2"; fnKeycode: Keycode.KEY_F2; numberKeycode: Keycode.KEY_2 }
        FnSymbolKey{ label: "3"; shiftedText: "#"; fnValue:"F3"; fnKeycode: Keycode.KEY_F3; numberKeycode: Keycode.KEY_3 }
        FnSymbolKey{ label: "4"; shiftedText: "$"; fnValue:"F4"; fnKeycode: Keycode.KEY_F4; numberKeycode: Keycode.KEY_4 }
        FnSymbolKey{ label: "5"; shiftedText: "%"; fnValue:"F5"; fnKeycode: Keycode.KEY_F5; numberKeycode: Keycode.KEY_5 }
        FnSymbolKey{ label: "6"; shiftedText: "^"; fnValue:"F6"; fnKeycode: Keycode.KEY_F6; numberKeycode: Keycode.KEY_6 }
        FnSymbolKey{ label: "7"; shiftedText: "&"; fnValue:"F7"; fnKeycode: Keycode.KEY_F7; numberKeycode: Keycode.KEY_7 }
        FnSymbolKey{ label: "8"; shiftedText: "*"; fnValue:"F8"; fnKeycode: Keycode.KEY_F8; numberKeycode: Keycode.KEY_8 }
        FnSymbolKey{ label: "9"; shiftedText: "("; fnValue:"F9"; fnKeycode: Keycode.KEY_F9; numberKeycode: Keycode.KEY_9 }
        FnSymbolKey{ label: "0"; shiftedText: ")"; fnValue:"F10"; fnKeycode: Keycode.KEY_F10; numberKeycode: Keycode.KEY_0 }
        FnSymbolKey{ label: "-"; shiftedText: "_"; fnValue:"F11"; fnKeycode: Keycode.KEY_F11; numberKeycode: Keycode.KEY_MINUS }
        FnSymbolKey{ label: "="; shiftedText: "+"; fnValue:"F12"; fnKeycode: Keycode.KEY_F12; numberKeycode: Keycode.KEY_EQUAL }
        BackspaceKey{ keycode: Keycode.KEY_BACKSPACE }
    }

    Row {
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: virtualKeyboard.keySpacing
        ActionKey{ label: "Tab"; width: virtualKeyboard.keyWidth*1.5 + virtualKeyboard.keySpacing; alignment: Text.AlignLeft; keycode: Keycode.KEY_TAB }
        LetterKey{ label: "q"; shiftedText: "Q"; keycode:  Keycode.KEY_Q }
        LetterKey{ label: "w"; shiftedText: "W"; keycode: Keycode.KEY_W }
        LetterKey{ label: "e"; shiftedText: "E"; keycode: Keycode.KEY_E }
        LetterKey{ label: "r"; shiftedText: "R"; keycode: Keycode.KEY_R }
        LetterKey{ label: "t"; shiftedText: "T"; keycode: Keycode.KEY_T }
        LetterKey{ label: "y"; shiftedText: "Y"; keycode: Keycode.KEY_Y }
        LetterKey{ label: "u"; shiftedText: "U"; keycode: Keycode.KEY_U }
        LetterKey{ label: "i"; shiftedText: "I"; keycode: Keycode.KEY_I }
        LetterKey{ label: "o"; shiftedText: "O"; keycode: Keycode.KEY_O }
        LetterKey{ label: "p"; shiftedText: "P"; keycode: Keycode.KEY_P }
        SymbolKey{ label: "["; shiftedText: "{"; keycode: Keycode.KEY_LEFTBRACE }
        SymbolKey{ label: "]"; shiftedText: "}"; keycode: Keycode.KEY_RIGHTBRACE }
        SymbolKey{ label: "\\"; shiftedText: "|"; keycode: Keycode.KEY_BACKSLASH }
        ActionKey{ label: "Del"; keycode: Keycode.KEY_DELETE }
    }
    Row {
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: virtualKeyboard.keySpacing

        CapslockKey{width: virtualKeyboard.keyWidth*2 + virtualKeyboard.keySpacing; alignment: Text.AlignLeft; keycode: Keycode.KEY_CAPSLOCK }
        LetterKey{ label: "a"; shiftedText: "A"; keycode: Keycode.KEY_A }
        LetterKey{ label: "s"; shiftedText: "S"; keycode: Keycode.KEY_S }
        LetterKey{ label: "d"; shiftedText: "D"; keycode: Keycode.KEY_D }
        LetterKey{ label: "f"; shiftedText: "F"; keycode: Keycode.KEY_F }
        LetterKey{ label: "g"; shiftedText: "G"; keycode: Keycode.KEY_G }
        LetterKey{ label: "h"; shiftedText: "H"; keycode: Keycode.KEY_H }
        LetterKey{ label: "j"; shiftedText: "J"; keycode: Keycode.KEY_J }
        LetterKey{ label: "k"; shiftedText: "K"; keycode: Keycode.KEY_K }
        LetterKey{ label: "l"; shiftedText: "L"; keycode: Keycode.KEY_L }
        SymbolKey{ label: ";"; shiftedText: ":"; keycode: Keycode.KEY_SEMICOLON }
        SymbolKey{ label: "'"; shiftedText: "\""; keycode: Keycode.KEY_APOSTROPHE }
        EnterKey{ keycode: Keycode.KEY_ENTER }

    }
    Row {
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: virtualKeyboard.keySpacing
        ShiftKey{width: virtualKeyboard.keyWidth*2.5 + virtualKeyboard.keySpacing*2; alignment: Text.AlignLeft; keycode: Keycode.KEY_LEFTSHIFT }
        LetterKey{ label: "z" ; shiftedText: "Z"; keycode: Keycode.KEY_Z }
        LetterKey{ label: "x" ; shiftedText: "X"; keycode: Keycode.KEY_X }
        LetterKey{ label: "c" ; shiftedText: "C"; keycode: Keycode.KEY_C }
        LetterKey{ label: "v" ; shiftedText: "V"; keycode: Keycode.KEY_V }
        LetterKey{ label: "b" ; shiftedText: "B"; keycode: Keycode.KEY_B }
        LetterKey{ label: "n" ; shiftedText: "N"; keycode: Keycode.KEY_N }
        LetterKey{ label: "m" ; shiftedText: "M"; keycode: Keycode.KEY_M }
        SymbolKey{ label: ", "; shiftedText: "<"; keycode: Keycode.KEY_COMMA }
        SymbolKey{ label: "."; shiftedText: ">"; keycode: Keycode.KEY_DOT }
        SymbolKey{ label: "/"; shiftedText: "?"; keycode: Keycode.KEY_SLASH }
        ActionKey{ label: "up"; actionKeyImgPath: "qrc:/img/up.svg"; keycode: Keycode.KEY_UP }
        ShiftKey{ width: virtualKeyboard.keyWidth*2 + virtualKeyboard.keySpacing; alignment: Text.AlignRight; keycode: Keycode.KEY_RIGHTSHIFT }
    }
    Row {
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: virtualKeyboard.keySpacing
        CtrlKey{ keycode: Keycode.KEY_LEFTCTRL }
        FnKey{}
        //暂时删除win键
//        WinKey{}
        AltKey{ keycode: Keycode.KEY_LEFTALT }
        ChangeImKey{}
        SpaceKey{ keycode: Keycode.KEY_SPACE }
        AltKey{ keycode: Keycode.KEY_RIGHTALT }
        ActionKey{ label: "left"; actionKeyImgPath: "qrc:/img/left.svg"; keycode: Keycode.KEY_LEFT }
        ActionKey{ label: "down"; actionKeyImgPath: "qrc:/img/down.svg"; keycode: Keycode.KEY_DOWN }
        ActionKey{ label: "right"; actionKeyImgPath: "qrc:/img/right.svg"; keycode: Keycode.KEY_RIGHT }
        CtrlKey{ keycode: Keycode.KEY_RIGHTCTRL }
    }
}

