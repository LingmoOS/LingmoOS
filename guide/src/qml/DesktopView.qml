/*
 * Copyright (C) 2024 LingmoOS Team.
 *
 * Author:     LingmoOS Team <team@lingmo.org>
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
import QtQuick.Layouts 1.12
import LingmoUI 1.0 as LingmoUI
import QtGraphicalEffects 1.0

Item {
    id: control

    Component {
        id: deviceItem

        Rectangle {
            id: d1

            width: win.width/10
            height: win.height/10
            color: LingmoUI.Theme.textColor
            border.width: 3
            radius: LingmoUI.Theme.bigRadius
        }
    }

    Component {
        id: wallpaperItem

        Image {
            id: image
            // source: "file://" + background.currentBackgroundPath
            source: "qrc:/assets/wallpaper/MundoLight.jpeg"
        }
    }

    Component {
        id: colorItem

        Rectangle {
            color: background.backgroundColor
        }
    }

    Loader {
        id: bgLoader
        anchors.fill: parent

        sourceComponent: {
            if (background.backgroundType === 0)
                return wallpaperItem

            return colorItem
        }
    }

    // Rectangle {
    //     id: desktopItem

    //     anchors.left: parent.left
    //     anchors.top: parent.top

    //     anchors.leftMargin: 10
    //     anchors.topMargin: 10

    //     width: 30
    //     height: width

    //     radius: height * 0.2
    //     opacity: 0.5
    //     color: LingmoUI.Theme.backgroundColor
    // }

    // Rectangle {
    //     id: desktopItem2

    //     anchors.left: parent.left
    //     anchors.top: desktopItem.bottom

    //     anchors.leftMargin: 10
    //     anchors.topMargin: 10

    //     width: 30
    //     height: width

    //     radius: height * 0.2
    //     opacity: 0.5
    //     color: LingmoUI.Theme.backgroundColor
    // }

    // Rectangle {
    //     id: dockArea
    //     anchors.left: parent.left
    //     anchors.right: parent.right
    //     anchors.bottom: parent.bottom

    //     anchors.leftMargin: 8
    //     anchors.rightMargin: 8
    //     anchors.bottomMargin: 8

    //     height: 38
    //     radius: height * 0.3
    //     color: LingmoUI.Theme.backgroundColor
    //     opacity: 0.7
    // }

    layer.enabled: true
    layer.effect: OpacityMask {
        maskSource: Item {
            width: control.width
            height: control.height

            Rectangle {
                anchors.fill: parent
                radius: LingmoUI.Theme.bigRadius
            }
        }
    }
}
