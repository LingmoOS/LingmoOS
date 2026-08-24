/*
 * Copyright (C) 2021 LingmoOS.
 *
 * Author:     Reion Wong <reionwong@gmail.com>
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
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtGraphicalEffects 1.0
import LingmoUI 1.0 as LingmoUI

ToolButton {
    id: root

    property int currentIndex: -1
    property int rootFontSize

    visible: menu.items.length > 1
    implicitHeight: _currentLabel.implicitHeight
    implicitWidth: _currentLabel.implicitWidth

    style: ButtonStyle {
        background: Rectangle {
            color: "transparent"
        }
    }

    Label {
        id: _currentLabel
        anchors.centerIn: parent
        color: "white"
        font.pointSize: rootFontSize
        text: instantiator.objectAt(currentIndex).text || ""
    }

    DropShadow {
        anchors.fill: _currentLabel
        source: _currentLabel
        z: -1
        horizontalOffset: 1
        verticalOffset: 1
        radius: 15
        samples: radius * 4
        spread: 0.35
        color: Qt.rgba(0, 0, 0, 0.2)
        opacity: 0.5
        visible: true
    }

    Component.onCompleted: {
        currentIndex = sessionModel.lastIndex
    }

    menu: Menu {
        id: menu
        Instantiator {
            id: instantiator
            model: sessionModel
            onObjectAdded: menu.insertItem(index, object)
            onObjectRemoved: menu.removeItem( object )
            delegate: MenuItem {
                text: model.name
                onTriggered: {
                    root.currentIndex = model.index
                }
            }
        }
    }
}
