/*
 * SPDX-FileCopyrightText: 2021 Reion Wong <reion@cuteos.com>
 * SPDX-FileCopyrightText: 2020 George Florea Bănuș <georgefb899@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtGraphicalEffects 1.12
import Qt.labs.platform 1.0 as Platform

import LingmoUI 1.0 as CuteUI
import mpv 1.0

CuteUI.Window {
    id: rootWindow
    width: 720
    height: 480
    minimumWidth: 700
    minimumHeight: 450
    color: CuteUI.Theme.backgroundColor

    header.visible: !rootWindow.isFullScreen()
    headerBackground.color: CuteUI.Theme.secondBackgroundColor
    // headerBackground.opacity: 0.95

    contentTopMargin: header.visible ? header.height : 0

    property int preFullScreenVisibility

    onVisibilityChanged: {
        if (!rootWindow.isFullScreen()) {
            preFullScreenVisibility = visibility
        }
    }

    // Window move
    DragHandler {
        target: null
        acceptedDevices: PointerDevice.GenericPointer
        grabPermissions: PointerHandler.CanTakeOverFromItems | PointerHandler.CanTakeOverFromHandlersOfDifferentType | PointerHandler.ApprovesTakeOverByAnything
        onActiveChanged: if (active) rootWindow.helper.startSystemMove(rootWindow)
    }

    headerItem: Item {
        Label {
            id: headerLabel
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.topMargin: CuteUI.Units.smallSpacing * 1.5
            anchors.leftMargin: CuteUI.Units.largeSpacing
            text: mpv.mediaTitle ? mpv.mediaTitle : qsTr("Video Player")
//            color: "white"
            color: CuteUI.Theme.textColor
            z: 100
        }

//        DropShadow {
//            anchors.fill: headerLabel
//            source: headerLabel
//            z: -1
//            horizontalOffset: 1
//            verticalOffset: 1
//            radius: Math.round(6 * CuteUI.Units.devicePixelRatio)
//            samples: radius * 2 + 1
//            spread: 0.35
//            color: Qt.rgba(0, 0, 0, 0.5)
//            opacity: 0.4
//            visible: true
//        }
    }

    Actions {
        id: actions
    }

    MpvVideo {
        id: mpv
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: !rootWindow.isFullScreen() ? footer.top : parent.bottom

        Image {
            id: _logo
            anchors.centerIn: parent
            width: 128
            height: 128
            source: "qrc:/images/lingmo-videoplayer.svg"
            sourceSize: Qt.size(width, height)
            visible: playList.playlistView.count === 0
        }

        Button {
            visible: _logo.visible
            anchors.top: _logo.bottom
            anchors.topMargin: CuteUI.Units.largeSpacing
            anchors.horizontalCenter: parent.horizontalCenter
            text: qsTr("Open")
            onClicked: fileDialog.open()
        }
    }

    Footer {
        id: footer
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
    }

    Platform.FileDialog {
        id: fileDialog

        property url location: ""

        folder: location
        title: "Select file"
        fileMode: Platform.FileDialog.OpenFile

        onAccepted: {
            openFile(fileDialog.file.toString(), true)
            // the timer scrolls the playlist to the playing file
            // once the table view rows are loaded
            playList.scrollPositionTimer.start()
            mpv.focus = true
        }
        onRejected: mpv.focus = true
    }

    PlayList {
        id: playList
    }

    function openFile(path, startPlayback) {
        mpv.playlistModel.addVideo(path)
        // mpv.playlistModel.clear()
        mpv.pause = !startPlayback
        // mpv.playlistModel.getVideos(path)
        mpv.loadFile(path)
    }

    function isFullScreen() {
        return rootWindow.visibility === Window.FullScreen
    }

    function toggleFullScreen() {
        if (!isFullScreen()) {
            rootWindow.showFullScreen()
        } else {
            if (rootWindow.preFullScreenVisibility === Window.Windowed) {
                rootWindow.showNormal()
            }
            if (rootWindow.preFullScreenVisibility === Window.Maximized) {
                rootWindow.show()
                rootWindow.showMaximized()
            }
        }
        app.showCursor()
        playList.scrollPositionTimer.start()
    }
}
