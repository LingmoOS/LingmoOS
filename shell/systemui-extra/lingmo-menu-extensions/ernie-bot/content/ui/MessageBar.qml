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
 * Authors: hxf <hewenfei@kylinos.cn>
 */

import QtQuick 2.12
import QtQuick.Controls 2.12

Item {
    id: root
    property double padding: 12

    // TODO: use enum
    property bool alignRight: true

    property alias color: messageBackgroud.color
    property alias border: messageBackgroud.border
    property alias radius: messageBackgroud.radius
    property alias messageColor: messageContent.color
    property alias selectionColor: messageContent.selectionColor

    readonly property Rectangle backgroud: messageBackgroud

    height: childrenRect.height

    // 更新内容宽高
    onWidthChanged: adjustContentWidth()
    Component.onCompleted: adjustContentWidth()

    Component {
        id: textComponent
        Text {}
    }

    function adjustContentWidth() {
        let text = null;
        let backgroundWidth = 0;

        if (textComponent.status === Component.Ready) {
            text = textComponent.createObject(null, {text: content});

            let contentWidth = Math.ceil(text.contentWidth);
            backgroundWidth = contentWidth > (width - padding*2) ? width : (contentWidth + padding*2);

            text.destroy();
        }

        messageBackgroud.width = backgroundWidth;
    }

    Rectangle {
        id: messageBackgroud
        anchors {
            left: root.alignRight ? undefined : parent.left
            right: root.alignRight ? parent.right : undefined
        }
        height: messageContent.contentHeight + padding * 2

        TextEdit {
            id: messageContent
            z: 100

            anchors.left: parent.left
            anchors.top: parent.top
            anchors.right: parent.right

            anchors.leftMargin: root.padding
            anchors.topMargin: root.padding
            anchors.rightMargin: root.padding

            text: content
            // textFormat: Text.AutoText
            textFormat: TextEdit.MarkdownText
            selectByMouse: true
            selectByKeyboard: true
            readOnly: true
            wrapMode: Text.WrapAnywhere
            onTextChanged: root.adjustContentWidth()
        }

        MouseArea {
            anchors.fill: parent
            z: 50
            acceptedButtons: Qt.RightButton | Qt.LeftButton
            onClicked: function(event) {
                if (event.button === Qt.LeftButton) {
                    messageContent.deselect();

                } else {
                    let start = messageContent.selectionStart;
                    let end = messageContent.selectionEnd;
                    if (messageContent.selectedText.length === 0) {
                        start = 0;
                        end = messageContent.length;
                    }

                    contextMenu.popup();
                    messageContent.select(start, end);

                    event.accepted = true;
                }
            }

            ContextMenu {
                id: contextMenu
                width: 120

                Action {
                    text: qsTr("Copy")
                    onTriggered: {
                        messageContent.copy();
                    }
                }
            }
        }
    }
}
