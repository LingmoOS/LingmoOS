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
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.5
import org.lingmo.menu.core 1.0
import org.lingmo.quick.platform 1.0 as Platform
import org.lingmo.quick.items 1.0 as LingmoUIItems

Item {
    id: contain
    property bool textCenterIn: false
    property bool editStatus: false
    property string textEdited: title
    property string text: editLoader.item.text
    property real textSize

    signal textEditingFinished(string text)
    Component {
        id: unEditText
        LingmoUIItems.StyleText {
            id: textShow
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: contain.textCenterIn ? Text.AlignHCenter : Text.AlignLeft
            elide: Text.ElideRight
            text: contain.textEdited
            paletteRole: Platform.Theme.Text
            font.bold: true
            font.pointSize: textSize

            MouseArea {
                id: textArea
                anchors.fill: parent
                onDoubleClicked: contain.editStatus = true;
            }
        }
    }

    Component {
        id: editText
        LingmoUIItems.StyleBackground {
            radius: Platform.Theme.normalRadius
            useStyleTransparency: false
            alpha: textEdit.activeFocus ? 0.04 : 0
            paletteRole: Platform.Theme.Text
            border.width: 2
            borderAlpha: textEdit.activeFocus ? 1 : 0
            borderColor: Platform.Theme.Highlight
            property string text: textEdit.text

            TextInput {
                id: textEdit
                clip: true
                anchors.left: parent.left
                anchors.leftMargin: 8
                anchors.right: buttonMouseArea.left
                anchors.rightMargin: 8
                anchors.verticalCenter: parent.verticalCenter
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: contain.textCenterIn ? Text.AlignHCenter : Text.AlignLeft
                text: contain.textEdited
                selectByMouse: true
                maximumLength: 14
                font.pointSize: textSize
                font.family: Platform.Theme.fontFamily;

                onEditingFinished: {
//                    modelManager.getFolderModel().renameFolder(text);
                    contain.textEdited = text;
                    contain.editStatus = false;
                    textEditingFinished(text);
                }

                function updateTextInputColor() {
                    color = Platform.Theme.text();
                    selectionColor = Platform.Theme.highlight();
                }

                Platform.Theme.onPaletteChanged: {
                    updateTextInputColor();
                }

                Component.onCompleted: {
                    updateTextInputColor();
                    forceActiveFocus();
                }

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.IBeamCursor
                    enabled: false
                }
            }

            MouseArea {
                id: buttonMouseArea
                hoverEnabled: true
                width: 16; height: width
                anchors.right: parent.right
                anchors.rightMargin: 10
                anchors.verticalCenter: parent.verticalCenter
                visible: textEdit.activeFocus

                LingmoUIItems.Icon {
                    anchors.centerIn: parent
                    width: 16; height: width
                    source: "image://theme/edit-clear-symbolic"
                    mode: LingmoUIItems.Icon.AutoHighlight
                }

                onClicked: {
                    textEdit.text = textEdited;
                    contain.editStatus = false;
                }
            }
        }
    }

    Loader {
        id: editLoader
        anchors.fill: parent
        sourceComponent: contain.editStatus ? editText : unEditText
    }
}
