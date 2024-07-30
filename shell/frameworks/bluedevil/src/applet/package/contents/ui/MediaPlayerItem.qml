/*
    SPDX-FileCopyrightText: 2015 David Rosca <nowrep@gmail.com>
    SPDX-FileCopyrightText: 2024 ivan tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

import org.kde.bluezqt as BluezQt
import org.kde.kirigami as Kirigami
import org.kde.plasma.components as PlasmaComponents3

ColumnLayout {
    id: root

    required property BluezQt.MediaPlayer mediaPlayer

    spacing: 0

    PlasmaComponents3.Label {
        id: trackTitleLabel
        Layout.fillWidth: true
        elide: Text.ElideRight
        font.weight: root.mediaPlayer !== null && root.mediaPlayer.track.title !== "" ? Font.DemiBold : Font.Normal
        font.italic: root.mediaPlayer !== null && root.mediaPlayer.status === BluezQt.MediaPlayer.Playing
        font.pointSize: Kirigami.Theme.smallFont.pointSize
        font.family: Kirigami.Theme.smallFont.family
        opacity: 0.6
        text: root.trackTitleText()
        textFormat: Text.PlainText
        visible: text.length
    }

    PlasmaComponents3.Label {
        id: trackArtistLabel
        Layout.fillWidth: true
        elide: Text.ElideRight
        font: Kirigami.Theme.smallFont
        opacity: 0.6
        text: root.mediaPlayer?.track.artist ?? ""
        textFormat: Text.PlainText
        visible: text.length
    }

    PlasmaComponents3.Label {
        id: trackAlbumLabel
        Layout.fillWidth: true
        elide: Text.ElideRight
        font: Kirigami.Theme.smallFont
        opacity: 0.6
        text: root.mediaPlayer?.track.album ?? ""
        textFormat: Text.PlainText
        visible: text.length
    }

    RowLayout {
        spacing: 0

        PlasmaComponents3.ToolButton {
            id: previousButton
            icon.name: "media-skip-backward-symbolic"

            onClicked: root.mediaPlayer?.previous()
        }

        PlasmaComponents3.ToolButton {
            id: playPauseButton
            icon.name: root.playPauseButtonIcon()

            onClicked: root.playPauseButtonClicked()
        }

        PlasmaComponents3.ToolButton {
            id: stopButton
            icon.name: "media-playback-stop-symbolic"
            enabled: root.mediaPlayer !== null && root.mediaPlayer.status !== BluezQt.MediaPlayer.Stopped

            onClicked: root.mediaPlayer?.stop()
        }

        PlasmaComponents3.ToolButton {
            id: nextButton
            icon.name: "media-skip-forward-symbolic"

            onClicked: root.mediaPlayer?.next()
        }
    }

    function trackTitleText(): string {
        if (!mediaPlayer) {
            return "";
        }

        const play = "\u25B6";

        if (mediaPlayer.status === BluezQt.MediaPlayer.Playing) {
            return "%1 %2".arg(play).arg(mediaPlayer.track.title);
        }
        return mediaPlayer.track.title;
    }

    function playPauseButtonIcon(): string {
        if (!mediaPlayer) {
            return "";
        }

        if (mediaPlayer.status !== BluezQt.MediaPlayer.Playing) {
            return "media-playback-start-symbolic";
        } else {
            return "media-playback-pause-symbolic";
        }
    }

    function playPauseButtonClicked(): void {
        if (mediaPlayer.status !== BluezQt.MediaPlayer.Playing) {
            mediaPlayer.play();
        } else {
            mediaPlayer.pause();
        }
    }
}
