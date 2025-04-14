/*
 * Copyright (C) 2025 Lingmo OS Team.
 *
 * Author:     Lingmo OS Team <team@lingmo.org>
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
import QtQuick.Window 2.12
import QtGraphicalEffects 1.0

import Lingmo.Desktop 1.0 as DK
import LingmoUI 1.0 as LingmoUI

Item {
    id: control

    DK.DesktopSettings {
        id: settings
    }

    Loader {
        id: backgroundLoader
        anchors.fill: parent
        anchors.margins: 0
        sourceComponent: settings.backgroundType === 0 ? wallpaper : background
    }

    Component {
        id: background

        Rectangle {
            anchors.fill: parent
            visible: settings.backgroundVisible
            color: settings.backgroundColor
        }
    }

    Component {
        id: wallpaper

        Image {
            source: "file://" + settings.wallpaper
            sourceSize: Qt.size(width * Screen.devicePixelRatio,
                                height * Screen.devicePixelRatio)
            fillMode: Image.PreserveAspectCrop
            clip: true
            visible: settings.backgroundVisible
            cache: false

            // Clear cache
            onSourceChanged: dirModel.clearPixmapCache()

            ColorOverlay {
                id: dimsWallpaper
                anchors.fill: parent
                source: parent
                color: "#000000"
                opacity: LingmoUI.Theme.darkMode && settings.dimsWallpaper ? 0.4 : 0.0

                Behavior on opacity {
                    NumberAnimation {
                        duration: 200
                    }
                }

            }
        }
    }

    // Text {
    //     id: desktopWatermark
    //     text: qsTr("%1\nLingmo OS Developer preview version").arg(settings.version)
    //     anchors.right: parent.right
    //     anchors.rightMargin: 10
    //     anchors.top: parent.top
    //     anchors.topMargin: 29
    //     font.pixelSize: 24 
    //     color: "#80FFFFFF" // 半透明颜色
    //     opacity: 0.76 // 透明度
    //     horizontalAlignment: Text.AlignRight
    //     renderType: Text.NativeRendering // 使用本地渲染以提高性能
    // }
}
