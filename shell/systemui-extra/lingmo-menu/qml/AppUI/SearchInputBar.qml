/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 */

import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.12
import org.lingmo.menu.core 1.0
import AppControls2 1.0 as AppControls2
import org.lingmo.quick.platform 1.0 as Platform
import org.lingmo.quick.items 1.0 as LingmoUIItems

LingmoUIItems.StyleBackground {
    property Item changeFocusTarget
    property alias text: textInput.text;

    alpha: 0.04
    useStyleTransparency: false
    paletteRole: Platform.Theme.Text
    border.width: 1
    borderAlpha: textInput.activeFocus ? 1 : 0.1
    borderColor: textInput.activeFocus ? Platform.Theme.Highlight : Platform.Theme.Base
    Component.onCompleted: mainWindow.visibleChanged.connect(clear)
    Component.onDestruction: mainWindow.visibleChanged.disconnect(clear)
    function textInputFocus() {
        textInput.forceActiveFocus();
    }
    function clear() {
        textInput.clear();
    }
    Item {
        id: defaultSearch
        width: searchIcon.width + defaultText.contentWidth; height: parent.height
        anchors.verticalCenter: parent.verticalCenter
        anchors.left: parent.left

        Item {
            id: searchIcon
            width: 32; height: 32
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.leftMargin: (parent.height - height) / 2

            LingmoUIItems.Icon {
                anchors.centerIn: parent
                width: parent.height / 2; height: width
                source: "search-symbolic"
                mode: LingmoUIItems.Icon.AutoHighlight
            }
        }


        LingmoUIItems.StyleText {
            id: defaultText
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: searchIcon.right
            text: qsTr("Search App")
            visible: textInput.contentWidth === 0
            paletteRole: Platform.Theme.Text
            verticalAlignment: TextInput.AlignVCenter
            alpha: 0.25
        }
    }

    TextInput {
        id: textInput
        clip: true
        anchors.right: clearButton.left
        width: parent.width - searchIcon.width - clearButton.width -
               searchIcon.anchors.leftMargin - clearButton.anchors.rightMargin
        height: parent.height
        selectByMouse: true
        verticalAlignment: TextInput.AlignVCenter
        font.pointSize: defaultText.font.pointSize
        font.family: defaultText.font.family
        focus: true
        activeFocusOnTab: true

        function changeFocusToListView() {
            if (!mainWindow.isFullScreen) {
                normalUI.focus = true;
                changeFocusTarget.focus = true;
                appPage.content.resetFocus();
            }
        }
        Keys.onDownPressed: changeFocusToListView()
        Keys.onReturnPressed: changeFocusToListView()

        function updateTextInputColor() {
            color = Platform.Theme.text();
            selectionColor = Platform.Theme.highlight();
        }

        Platform.Theme.onPaletteChanged: {
            updateTextInputColor();
        }

        Component.onCompleted: {
            updateTextInputColor();
        }
    }

    LingmoUIItems.Button {
        id: clearButton
        anchors.right: parent.right
        anchors.rightMargin: (parent.height - height) / 2
        anchors.verticalCenter: parent.verticalCenter
        width: 32; height: width
        background.radius: width / 2
        visible: textInput.length !== 0
        activeFocusOnTab: false

        icon.source: "edit-clear-symbolic"
        icon.mode: LingmoUIItems.Icon.AutoHighlight

        onClicked: {
            textInput.clear();
        }
    }

    MouseArea {
        anchors.left: textInput.left
        anchors.verticalCenter: parent.verticalCenter
        width: clearButton.visible ? textInput.width : textInput.width + clearButton.width
        height: parent.height
        cursorShape: Qt.IBeamCursor
        enabled: false
    }
}
