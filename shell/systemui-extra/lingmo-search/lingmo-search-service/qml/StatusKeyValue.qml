/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: iaom <zhangpengfei@kylinos.cn>
 *
 */
import QtQuick 2.0

Row {
    id: kvRow
    width: parent.width
    height: childrenRect.height
    property string keyText : "--"
    property string valueText :"--"
    Text {
        id: keyTextLabel
        anchors.left: parent.Left
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter
        font.family: "stxihei"
        font.pixelSize: 16
        color: "black"
        text: kvRow.keyText
    }
    Text {
        id: valueTextLabel
        anchors.left: parent.Right
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter
        font.family: "stxihei"
        font.pixelSize: 16
        color: "blue"
        text: kvRow.valueText
    }
    onKeyTextChanged: {
        keyTextLabel.text = keyText
    }
    onValueTextChanged: {
        valueTextLabel.text =  valueText
    }
}

