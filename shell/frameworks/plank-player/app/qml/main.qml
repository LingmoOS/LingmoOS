// SPDX-FileCopyrightText: 2021 Aditya Mehra <aix.m@outlook.com>
//
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Layouts 1.15
import org.kde.kirigami 2.19 as Kirigami
import QtQuick.Controls 2.15 as Controls
import QtMultimedia

Kirigami.AbstractApplicationWindow {
    id: window
    visible: true
    title: i18n("PlankPlayer")
    color: Kirigami.Theme.backgroundColor
    width:  Screen.desktopAvailableWidth
    height: Screen.desktopAvailableHeight
    visibility: "FullScreen"
    property var videoSource: argumentFileUrl ? argumentFileUrl : ""

    Component.onCompleted: {
        console.log(HomeDirectory)
        playerOSDItem.opened = true
        if(videoSource !== "") {
            video.source = videoSource
            video.play()
        }
    }

    onVideoSourceChanged: {
        if(videoSource !== ""){
            video.source = Qt.resolvedUrl(videoSource)
            playerOSDItem.opened = true
            video.play()
        }
    }

    Rectangle {
        anchors.fill: parent
        color: "black"

        Menu {
            id: mainMenu
            onClosed: {
                playerOSDItem.opened = true
            }
        }

        Video {
            id: video
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: playerOSDItem.opened ? playerOSDItem.top : parent.bottom
            focus: true

            Keys.onDownPressed: (event)=> {
                playerOSDItem.opened = true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: (mouse)=> {
                    if(!playerOSDItem.pinned){
                       playerOSDItem.opened = !playerOSDItem.opened
                    }
                }
            }

            onPlaybackStateChanged: {
                if(video.playbackState == MediaPlayer.StoppedState){
                    video.stop()
                }
            }
        }

        PlayerOSD {
            id: playerOSDItem
            anchors.bottom: parent.bottom
            height: parent.height * 0.10
            videoItem: video
            menuOpened: mainMenu.opened
        }
    }
}
