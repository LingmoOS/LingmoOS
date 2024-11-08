/*
 * Copyright (C) 2021 LingmoOS Team.
 *
 * Author:     revenmartin <revenmartin@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.12
import QtQuick.Controls 2.12
import QtGraphicalEffects 1.0

import Lingmo.FileManager 1.0
import LingmoUI 1.0 as LingmoUI

Item {
    id: control

    property string url: ""

    signal itemClicked(string path)
    signal editorAccepted(string path)

    Rectangle {
        anchors.fill: parent
        color: LingmoUI.Theme.darkMode ? Qt.lighter(LingmoUI.Theme.secondBackgroundColor, 1.3)
                                     : LingmoUI.Theme.secondBackgroundColor
        radius: LingmoUI.Theme.smallRadius
        z: -1
    }

    ListView {
        id: _pathView
        anchors.fill: parent
        anchors.topMargin: 2
        anchors.bottomMargin: 2
        model: _pathBarModel
        orientation: Qt.Horizontal
        layoutDirection: Qt.LeftToRight
        clip: true

        leftMargin: 3
        rightMargin: 3
        spacing: LingmoUI.Units.smallSpacing

        onCountChanged: {
            _pathView.currentIndex = _pathView.count - 1
            _pathView.positionViewAtEnd()
        }

        MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.LeftButton
            onClicked: openEditor()
            z: -1
        }

        highlight: Rectangle {
            radius: LingmoUI.Theme.smallRadius
            color: Qt.rgba(LingmoUI.Theme.highlightColor.r,
                           LingmoUI.Theme.highlightColor.g,
                           LingmoUI.Theme.highlightColor.b, LingmoUI.Theme.darkMode ? 0.3 : 0.1)
            smooth: true
        }

        delegate: MouseArea {
            id: _item
            height: ListView.view.height - ListView.view.topMargin - ListView.view.bottomMargin
            width: _name.width + LingmoUI.Units.largeSpacing
            hoverEnabled: true
            z: -1

            property bool selected: index === _pathView.count - 1

            onClicked: control.itemClicked(model.path)

            Rectangle {
                anchors.fill: parent
                radius: LingmoUI.Theme.smallRadius
                color: _item.pressed ? Qt.rgba(LingmoUI.Theme.textColor.r,
                                                   LingmoUI.Theme.textColor.g,
                                                   LingmoUI.Theme.textColor.b, LingmoUI.Theme.darkMode ? 0.05 : 0.1) :
                       _item.containsMouse ? Qt.rgba(LingmoUI.Theme.textColor.r,
                                                     LingmoUI.Theme.textColor.g,
                                                     LingmoUI.Theme.textColor.b, LingmoUI.Theme.darkMode ? 0.1 : 0.05) :
                                                              "transparent"

                smooth: true
            }

            Label {
                id: _name
                text: model.name
                anchors.centerIn: parent
                color: selected ? LingmoUI.Theme.highlightColor : LingmoUI.Theme.textColor
            }
        }
    }

    TextField {
        id: _pathEditor
        anchors.fill: parent
        visible: false
        selectByMouse: true
        inputMethodHints: Qt.ImhUrlCharactersOnly | Qt.ImhNoAutoUppercase

        text: _pathBarModel.url
        color: LingmoUI.Theme.darkMode ? "white" : "black"

        background: Rectangle {
            radius: LingmoUI.Theme.smallRadius
            color: LingmoUI.Theme.darkMode ? Qt.lighter(LingmoUI.Theme.secondBackgroundColor, 1.7)
                                         : LingmoUI.Theme.secondBackgroundColor
            border.width: 1
            border.color: LingmoUI.Theme.highlightColor
        }

        onAccepted: {
            control.editorAccepted(text)
            closeEditor()
        }

        Keys.onPressed: {
            if (event.key === Qt.Key_Escape)
                focus = false
        }

        onActiveFocusChanged: {
            if (!activeFocus) {
                closeEditor()
            }
        }
    }

    PathBarModel {
        id: _pathBarModel
    }

    function updateUrl(url) {
        control.url = url
        _pathBarModel.url = url
    }

    function openEditor() {
        _pathEditor.text = _pathBarModel.url
        _pathEditor.visible = true
        _pathEditor.forceActiveFocus()
        _pathEditor.selectAll()
        _pathView.visible = false
    }

    function closeEditor() {
        _pathEditor.visible = false
        _pathView.visible = true
    }
}
