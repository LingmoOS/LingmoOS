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
import QtQuick.Window 2.12
import QtQuick.Controls 2.12
import QtGraphicalEffects 1.0
import QtQuick.Layouts 1.12
import LingmoUI 1.0 as LingmoUI
import Lingmo.System 1.0 as System
import Lingmo.Setup 1.0

Window {
    id: win
    visible: true
    visibility: Window.isFullScreen ? Window.Windowed : Window.FullScreen
    flags: Qt.Window & Qt.FramelessWindowHint
    title: qsTr("Welcome to Lingmo OS")

    System.Wallpaper {
        id: wallpaper
    }

    Image {
        id: wallpaperImage
        anchors.fill: parent
        source: "file://" + wallpaper.path
        sourceSize: Qt.size(width * Screen.devicePixelRatio,
                            height * Screen.devicePixelRatio)
        fillMode: Image.PreserveAspectCrop
        asynchronous: false
        clip: true
        cache: false
        smooth: true
    }

    FastBlur {
        anchors.fill: wallpaperImage
        source: wallpaperImage
        radius: 50
    }

    StackView {
        id: stackView
        anchors.fill: parent
        initialItem: homePage
    }

    SysVersion {
        id: info
    }

    Component {
        id: homePage

        HomePage { }
    }

    // Component {
    //     id: userAssets

    //     UserAssets {}
    // }

    Component {
        id: firstPage

        FirstPage {}
    }

    Component {
        id: secondPage

        SecondPage {}
    }

    Component {
        id: donePage

        DonePage {}
    }
}