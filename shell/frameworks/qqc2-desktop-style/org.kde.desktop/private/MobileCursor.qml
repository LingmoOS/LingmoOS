/*
    SPDX-FileCopyrightText: 2018 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

pragma ComponentBehavior: Bound

import QtQuick
import org.kde.kirigami as Kirigami
import org.kde.desktop.private as Private

Item {
    id: root

    width: 1 //<-important that this is actually a single device pixel
    height: Kirigami.Units.gridUnit

    property /*TextInput | TextEdit*/ Item target

    property bool selectionStartHandle: false

    visible: Kirigami.Settings.tabletMode && ((target.activeFocus && !selectionStartHandle) || target.selectedText.length > 0)

    Rectangle {
        width: 3
        anchors {
            horizontalCenter: parent.horizontalCenter
            top: parent.top
            bottom: parent.bottom
        }
        color: Qt.tint(Kirigami.Theme.highlightColor, Qt.rgba(1,1,1,0.4))
        radius: width
        Rectangle {
            width: Math.round(Kirigami.Units.gridUnit/1.5)
            height: width
            visible: Private.MobileTextActionsToolBar.shouldBeVisible
            anchors {
                horizontalCenter: parent.horizontalCenter
                verticalCenter: parent.bottom
            }
            radius: width
            color: Qt.tint(Kirigami.Theme.highlightColor, Qt.rgba(1,1,1,0.4))
        }
        MouseArea {
            anchors {
                fill: parent
                margins: -Kirigami.Units.gridUnit
            }
            preventStealing: true
            onPositionChanged: mouse => {
                const target = root.target;
                var pos = mapToItem(target, mouse.x, mouse.y);
                pos = target.positionAt(pos.x, pos.y);

                if (target.selectedText.length > 0) {
                    if (root.selectionStartHandle) {
                        target.select(Math.min(pos, target.selectionEnd - 1), target.selectionEnd);
                    } else {
                        target.select(target.selectionStart, Math.max(pos, target.selectionStart + 1));
                    }
                } else {
                    target.cursorPosition = pos;
                }
            }
        }
    }
}

