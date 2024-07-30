// SPDX-FileCopyrightText: 2021 Aditya Mehra <aix.m@outlook.com>
//
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Layouts 1.15
import org.kde.kirigami 2.15 as Kirigami
import QtQuick.Controls 2.15 as Controls
import QtMultimedia

Item {
    id: osdControlsArea
    anchors.left: parent.left
    anchors.right: parent.right
    property bool opened: false
    property bool menuOpened
    property var videoItem
    property bool pinned: false

    onOpenedChanged: {
        if(opened){
            menuButton.forceActiveFocus()
            hideTimer.restart()
        }
    }

    Timer {
        id: hideTimer
        interval: 5000
        onTriggered: {
            if(!osdControlsArea.pinned){
                osdControlsArea.opened = false;
                if(!menuOpened){
                    videoItem.forceActiveFocus();
                }
            }
        }
    }

    Rectangle {
        width: parent.width
        height: parent.height
        color: Qt.rgba(Kirigami.Theme.backgroundColor.r, Kirigami.Theme.backgroundColor.g, Kirigami.Theme.backgroundColor.b, 0.6)
        y: opened ? 0 : parent.height

        Behavior on y {
            YAnimator {
                duration: Kirigami.Units.longDuration
                easing.type: Easing.OutCubic
            }
        }

        RowLayout {
            id: mainOSDLayout
            anchors.fill: parent
            anchors.margins: Kirigami.Units.largeSpacing

            OSDButton {
                id: menuButton
                Layout.maximumWidth: Kirigami.Units.iconSizes.large
                Layout.preferredHeight: Kirigami.Units.iconSizes.large
                iconSource: "menu_new"
                KeyNavigation.right: playPauseButton

                onClicked: (mouse)=> {
                    mainMenu.open()
                }
            }

            OSDButton {
                id: playPauseButton
                Layout.maximumWidth: Kirigami.Units.iconSizes.large
                Layout.preferredHeight: Kirigami.Units.iconSizes.large
                iconSource: videoItem.playbackState === MediaPlayer.PlayingState ? "media-playback-pause"  : "media-playback-start"
                KeyNavigation.right: mediaSkipBckButton
                KeyNavigation.left: menuButton

                onClicked: (mouse)=> {
                    videoItem.playbackState === MediaPlayer.PlayingState ? videoItem.pause() : videoItem.play()
                }
            }

            OSDButton {
                id: mediaSkipBckButton
                Layout.maximumWidth: Kirigami.Units.iconSizes.large
                Layout.preferredHeight: Kirigami.Units.iconSizes.large
                iconSource: "media-seek-backward"
                KeyNavigation.right: mediaSkipFwdButton
                KeyNavigation.left: playPauseButton

                onClicked: (mouse)=> {
                    videoItem.seek(videoItem.position - 5000)
                }
            }

            OSDButton {
                id: mediaSkipFwdButton
                Layout.maximumWidth: Kirigami.Units.iconSizes.large
                Layout.preferredHeight: Kirigami.Units.iconSizes.large
                iconSource: "media-seek-forward"
                KeyNavigation.right: osdSeekBar
                KeyNavigation.left: mediaSkipBckButton

                onClicked: (mouse)=> {
                    videoItem.seek(videoItem.position + 5000)
                }
            }

            SeekBar {
                id: osdSeekBar
                Layout.fillWidth: true
                Layout.fillHeight: true
                KeyNavigation.right: pinOsdButton
                KeyNavigation.left: mediaSkipFwdButton
                duration: videoItem.duration
                value: videoItem.position
            }

            OSDButton {
                id: pinOsdButton
                Layout.maximumWidth: Kirigami.Units.iconSizes.large
                Layout.preferredHeight: Kirigami.Units.iconSizes.large
                iconSource: "unlock"
                KeyNavigation.right: fitVidOsdButton
                KeyNavigation.left: osdSeekBar

                onClicked: (mouse)=> {
                    console.log(osdControlsArea.pinned)
                    if(!osdControlsArea.pinned){
                        pinOsdButton.iconSource = "lock"
                        osdControlsArea.pinned = true
                    } else {
                        pinOsdButton.iconSource = "unlock"
                        osdControlsArea.pinned = false
                    }
                }
            }

            OSDButton {
                id: fitVidOsdButton
                Layout.maximumWidth: Kirigami.Units.iconSizes.large
                Layout.preferredHeight: Kirigami.Units.iconSizes.large
                iconSource: "zoom-fit-best"
                checkable: true
                KeyNavigation.right: exitPlayerButton
                KeyNavigation.left: pinOsdButton

                onClicked: (mouse)=> {
                    if(videoItem.fillMode == VideoOutput.PreserveAspectFit){
                        videoItem.fillMode = VideoOutput.Stretch
                    } else {
                        videoItem.fillMode = VideoOutput.PreserveAspectFit
                    }
                }
            }

            OSDButton {
                id: exitPlayerButton
                Layout.maximumWidth: Kirigami.Units.iconSizes.large
                Layout.preferredHeight: Kirigami.Units.iconSizes.large
                iconSource: "window-close"
                KeyNavigation.left: fitVidOsdButton

                onClicked: (mouse)=> {
                    window.close()
                }
            }
        }
    }
}
