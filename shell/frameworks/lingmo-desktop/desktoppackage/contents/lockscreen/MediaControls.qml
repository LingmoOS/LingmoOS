/*
    SPDX-FileCopyrightText: 2016 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts

import org.kde.lingmo.components 3.0 as LingmoComponents3
import org.kde.lingmo.extras 2.0 as LingmoExtras
import org.kde.lingmoui 2 as LingmoUI
import org.kde.lingmo.private.mpris as Mpris

Item {
    visible: instantiator.count > 0
    implicitHeight: LingmoUI.Units.gridUnit * 3
    implicitWidth: LingmoUI.Units.gridUnit * 16

    Repeater {
        id: instantiator
        model: Mpris.MultiplexerModel { }

        RowLayout {
            id: controlsRow

            anchors.fill: parent
            spacing: 0
            enabled: model.canControl

            Image {
                id: albumArt
                Layout.preferredWidth: height
                Layout.fillHeight: true
                visible: status === Image.Loading || status === Image.Ready
                asynchronous: true
                fillMode: Image.PreserveAspectFit
                source: model.artUrl
                sourceSize.height: height * Screen.devicePixelRatio
            }

            Item { // spacer
                width: LingmoUI.Units.smallSpacing
                height: 1
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 0

                LingmoComponents3.Label {
                    Layout.fillWidth: true
                    elide: Text.ElideRight
                    font.pointSize: LingmoUI.Theme.defaultFont.pointSize + 1
                    maximumLineCount: 1
                    text: model.track.length > 0
                            ? model.track
                            : (model.playbackStatus > Mpris.PlaybackStatus.Stopped
                                ? i18nd("lingmo_shell_org.kde.lingmo.desktop", "No title")
                                : i18nd("lingmo_shell_org.kde.lingmo.desktop", "No media playing"))
                    textFormat: Text.PlainText
                    wrapMode: Text.NoWrap
                }

                LingmoExtras.DescriptiveLabel {
                    Layout.fillWidth: true
                    wrapMode: Text.NoWrap
                    elide: Text.ElideRight
                    // if no artist is given, show player name instead
                    text: model.artist || model.identity
                    textFormat: Text.PlainText
                    font.pointSize: LingmoUI.Theme.smallFont.pointSize + 1
                    maximumLineCount: 1
                }
            }

            LingmoComponents3.ToolButton {
                Layout.preferredHeight: LingmoUI.Units.gridUnit * 2
                Layout.preferredWidth: Layout.preferredHeight
                visible: model.canGoBack || model.canGoNext
                enabled: model.canGoPrevious
                focusPolicy: Qt.TabFocus
                icon.name: LayoutMirroring.enabled ? "media-skip-forward" : "media-skip-backward"
                onClicked: {
                    fadeoutTimer.running = false
                    model.container.Previous()
                }
                Accessible.name: i18nd("lingmo_shell_org.kde.lingmo.desktop", "Previous track")
            }

            LingmoComponents3.ToolButton {
                Layout.fillHeight: true
                Layout.preferredWidth: height // make this button bigger
                focusPolicy: Qt.TabFocus
                icon.name: model.playbackStatus === Mpris.PlaybackStatus.Playing ? "media-playback-pause" : "media-playback-start"
                onClicked: {
                    fadeoutTimer.running = false
                    model.container.PlayPause()
                }
                Accessible.name: i18nd("lingmo_shell_org.kde.lingmo.desktop", "Play or Pause media")
            }

            LingmoComponents3.ToolButton {
                Layout.preferredHeight: LingmoUI.Units.gridUnit * 2
                Layout.preferredWidth: Layout.preferredHeight
                visible: model.canGoBack || model.canGoNext
                enabled: model.canGoNext
                focusPolicy: Qt.TabFocus
                icon.name: LayoutMirroring.enabled ? "media-skip-backward" : "media-skip-forward"
                Accessible.name: i18nd("lingmo_shell_org.kde.lingmo.desktop", "Next track")
                onClicked: {
                    fadeoutTimer.running = false
                    model.container.Next()
                }
            }
        }
    }
}
