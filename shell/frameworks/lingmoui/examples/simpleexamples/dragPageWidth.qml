/*
 *  SPDX-FileCopyrightText: 2017 Eike Hein <hein@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import org.kde.lingmoui as LingmoUI

LingmoUI.ApplicationWindow {
    id: root

    property int defaultColumnWidth: LingmoUI.Units.gridUnit * 13
    property int columnWidth: defaultColumnWidth

    pageStack.defaultColumnWidth: columnWidth
    pageStack.initialPage: [firstPageComponent, secondPageComponent]

    MouseArea {
        id: dragHandle

        visible: pageStack.wideMode

        anchors.top: parent.top
        anchors.bottom: parent.bottom

        x: columnWidth - (width / 2)
        width: 2

        property int dragRange: (LingmoUI.Units.gridUnit * 5)
        property int _lastX: -1

        cursorShape: Qt.SplitHCursor

        onPressed: mouse => {
            _lastX = mouse.x;
        }

        onPositionChanged: mouse => {
            if (mouse.x > _lastX) {
                columnWidth = Math.min((defaultColumnWidth + dragRange),
                    columnWidth + (mouse.x - _lastX));
            } else if (mouse.x < _lastX) {
                columnWidth = Math.max((defaultColumnWidth - dragRange),
                    columnWidth - (_lastX - mouse.x));
            }
        }

        Rectangle {
            anchors.fill: parent

            color: "blue"
        }
    }

    Component {
        id: firstPageComponent

        LingmoUI.Page {
            id: firstPage

            background: Rectangle { color: "red" }
        }
    }

    Component {
        id: secondPageComponent

        LingmoUI.Page {
            id: secondPage

            background: Rectangle { color: "green" }
        }
    }
}
