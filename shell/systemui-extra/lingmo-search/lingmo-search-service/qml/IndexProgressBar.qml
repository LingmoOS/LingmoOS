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
import QtQuick.Controls 2.5

Column {
    id: progressBar
    width: parent.width
    property var from: 0
    property var to: 0
    property var value: 0
    property string name: ""
    property string state: ""

    Row {
        Text {
            id: progressName
            anchors.left: parent.Left
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
            font.family: "stxihei"
            font.pixelSize: 14
            color: "black"
            text: progressBar.name
        }
        Text {
            id: progressState
            anchors.left: parent.Right
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
            font.family: "stxihei"
            font.pixelSize: 14
            color: "blue"
            text: progressBar.state
        }
    }


    Row {
        width: parent.width
        ProgressBar {
            width: parent.width * 2 / 3
            height: 16
            from: progressBar.from
            to: progressBar.to
            value: progressBar.value
        }
        Text {
            id: progressNum
            anchors.left: parent.Right
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
            font.family: "stxihei"
            font.pixelSize: 10
            color: "blue"

            text: "--/--"
        }
    }

    Component.onCompleted: {
        updateProgressNum();

    }
    onToChanged: {
        updateProgressNum();
    }

    onValueChanged: {
        updateProgressNum();
    }

    onNameChanged: {
        progressName.text = progressBar.name;
    }

    onStateChanged: {
        progressState.text = progressBar.state;
    }

    function updateProgressNum() {
        var str = "%1/%2";
        progressNum.text = str.arg(progressBar.value).arg(progressBar.to);
    }
}



