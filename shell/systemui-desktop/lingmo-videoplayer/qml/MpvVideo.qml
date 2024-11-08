/*
 * SPDX-FileCopyrightText: 2020 George Florea Bănuș <georgefb899@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import QtQuick 2.12
import QtQuick.Window 2.12
import LingmoUI 1.0 as CuteUI
import mpv 1.0

MpvObject {
    id: control

    property int mouseX: mouseArea.mouseX
    property int mouseY: mouseArea.mouseY

    onReady: {
        if (app.argument(0) !== "") {
            rootWindow.openFile(app.argument(0), true)
        }
    }

    onFileLoaded: {
        // loadingIndicatorParent.visible = false
        // header.audioTracks = getProperty("track-list").filter(track => track["type"] === "audio")
        // header.subtitleTracks = getProperty("track-list").filter(track => track["type"] === "sub")

        if (playList.playlistView.count <= 1) {
            setProperty("loop-file", "inf")
        }

        setProperty("ab-loop-a", "no")
        setProperty("ab-loop-b", "no")

        mpv.pause = loadTimePosition() !== 0
        position = loadTimePosition()
    }

    onEndFile: {
        if (reason === "error") {
            if (playlistModel.rowCount() === 0) {
                return
            }

            // osd.message(i18n("Could not play: %1").arg(title))
        }
        const nextFileRow = playlistModel.getPlayingVideo() + 1
        if (nextFileRow < playList.playlistView.count) {
            const nextFile = playlistModel.getPath(nextFileRow)
            playlistModel.setPlayingVideo(nextFileRow)
            loadFile(nextFile)
        } else {
            // Last file in playlist
            if (PlaylistSettings.repeat) {
                playlistModel.setPlayingVideo(0)
                loadFile(playlistModel.getPath(0))
            }
        }
    }

    onPauseChanged: {
        if (pause) {
            lockManager.setInhibitionOff()
        } else {
            lockManager.setInhibitionOn()
        }
    }

    Timer {
        id: hideCursorTimer

        property double tx: mouseArea.mouseX
        property double ty: mouseArea.mouseY
        property int timeNotMoved: 0

        running: rootWindow.isFullScreen() && mouseArea.containsMouse
        repeat: true
        interval: 50

        onTriggered: {
            if (mouseArea.mouseX === tx && mouseArea.mouseY === ty) {
                if (timeNotMoved > 2000) {
                    app.hideCursor()
                }
            } else {
                app.showCursor()
                timeNotMoved = 0
            }
            tx = mouseArea.mouseX
            ty = mouseArea.mouseY
            timeNotMoved += interval
        }
    }

    Connections {
        target: mediaPlayer2Player

        function onPlaypause() {
            actions.playPauseAction.trigger()
        }

        function onPlay() {
            control.pause = false
        }

        function onPause() {
            control.pause = true
        }

        function onStop() {
            control.position = 0
            control.pause = true
        }

        function onNext() {
            actions.playNextAction.trigger()
        }

        function onPrevious() {
            actions.playPreviousAction.trigger()
        }

        function onSeek() {
            control.command(["add", "time-pos", offset])
        }

        function onOpenUri() {
            control.command(["add", "time-pos", offset])
        }
    }

    Component.onCompleted: {
        mediaPlayer2Player.mpv = control
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        acceptedButtons: Qt.LeftButton | Qt.RightButton | Qt.MiddleButton

        onDoubleClicked: {
            if (mouse.button == Qt.LeftButton)
                rootWindow.toggleFullScreen()
        }

        onClicked: {
            if (mouse.button == Qt.RightButton)
                actions.playPauseAction.triggered()
        }

        onPressed: {
            playList.state = "hidden"
        }
    }

    DropArea {
        id: dropArea
        anchors.fill: parent
        keys: ["text/uri-list"]

        property var acceptedSubtitleTypes: ["application/x-subrip", "text/x-ssa"]

        onDropped: {
            if (acceptedSubtitleTypes.includes(app.mimeType(drop.urls[0]))) {
                const subFile = drop.urls[0].replace("file://", "")
                command(["sub-add", drop.urls[0], "select"])
            }

            if (app.mimeType(drop.urls[0]).startsWith("video/") || app.mimeType(drop.urls[0]).startsWith("audio/")) {
                rootWindow.openFile(drop.urls[0], true)
            }
        }
    }
}
