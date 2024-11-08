/*
 * SPDX-FileCopyrightText: 2020 George Florea Bănuș <georgefb899@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import QtQuick 2.12
import QtQuick.Controls 2.12

QtObject {
    id: root

    property var list: ({})

    property Action togglePlaylistAction: Action {
        id: togglePlaylistAction

        Component.onCompleted: list["togglePlaylistAction"] = togglePlaylistAction

        onTriggered: {
            if (playList.state === "visible") {
                playList.state = "hidden"
            } else {
                playList.state = "visible"
            }
        }
    }

    property Action playPreviousAction: Action {
        id: playPreviousAction
//        property var qaction: app.action("playPrevious")
//        text: qaction.text
//        shortcut: qaction.shortcutName
//        icon.name: qaction.iconName()

        Component.onCompleted: list["playPreviousAction"] = playPreviousAction

        onTriggered: {
            if (mpv.playlistModel.getPlayingVideo() !== 0) {
                const previousFileRow = mpv.playlistModel.getPlayingVideo() - 1
                const previousFile = mpv.playlistModel.getPath(previousFileRow)
                const updateLastPlayedFile = !playList.isYouTubePlaylist
                mpv.playlistModel.setPlayingVideo(previousFileRow)
                mpv.loadFile(previousFile, updateLastPlayedFile)
            }
        }
    }


    property Action muteAction: Action {
        id: muteAction
//        property var qaction: app.action("mute")
//        text: qaction.text
//        shortcut: qaction.shortcutName
//        icon.name: qaction.iconName()

        Component.onCompleted: list["muteAction"] = muteAction

        onTriggered: {
            mpv.setProperty("mute", !mpv.getProperty("mute"))
            if (mpv.getProperty("mute")) {
                icon.name = "player-volume-muted"
            } /*else {
                text = qaction.text
                icon.name = qaction.iconName()
            }*/
        }
    }

    property Action playNextAction: Action {
        id: playNextAction
//        property var qaction: app.action("playNext")
//        text: qaction.text
//        shortcut: qaction.shortcutName
//        icon.name: qaction.iconName()

        Component.onCompleted: list["playNextAction"] = playNextAction

        onTriggered: {
            const nextFileRow = mpv.playlistModel.getPlayingVideo() + 1
            const updateLastPlayedFile = !playList.isYouTubePlaylist
            if (nextFileRow < playList.playlistView.count) {
                const nextFile = mpv.playlistModel.getPath(nextFileRow)
                mpv.playlistModel.setPlayingVideo(nextFileRow)
                mpv.loadFile(nextFile, updateLastPlayedFile)
            } else {
                // Last file in playlist
                // if (PlaylistSettings.repeat) {
                    mpv.playlistModel.setPlayingVideo(0)
                    mpv.loadFile(mpv.playlistModel.getPath(0), updateLastPlayedFile)
                // }
            }
        }
    }

    property Action playPauseAction: Action {
        id: playPauseAction
        icon.name: "media-playback-pause"
        shortcut: "Space"

        Component.onCompleted: list["playPauseAction"] = playPauseAction

        onTriggered: {
            if (playList.playlistView.count === 0) {
                fileDialog.open()
                return
            }

            mpv.setProperty("pause", !mpv.getProperty("pause"))
        }
    }
}
