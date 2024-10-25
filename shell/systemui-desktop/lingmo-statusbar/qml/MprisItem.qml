/*
 * Copyright (C) 2024 LingmoOS Team.
 *
 * Author:     Reion Wong <aj@lingmo.org>
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
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12
import QtGraphicalEffects 1.0
import LingmoUI 1.0 as LingmoUI
import Lingmo.Mpris 1.0

Item {
    id: control

    property bool available: mprisManager.availableServices.length > 0
    property bool isPlaying: currentService && mprisManager.playbackStatus === Mpris.Playing
    property alias currentService: mprisManager.currentService
    property var artUrlTag: Mpris.metadataToString(Mpris.ArtUrl)
    property var titleTag: Mpris.metadataToString(Mpris.Title)
    property var artistTag: Mpris.metadataToString(Mpris.Artist)

    MprisManager {
        id: mprisManager

        onCurrentServiceChanged: {
            control.updateInfo()
        }

        onMetadataChanged: {
            control.updateInfo()
        }
    }

    Component.onCompleted: { control.updateInfo() }

    function updateInfo() {
        var titleAvailable = (titleTag in mprisManager.metadata) ? mprisManager.metadata[titleTag].toString() !== "" : false
        var artistAvailable = (artistTag in mprisManager.metadata) ? mprisManager.metadata[artistTag].toString() !== "" : false

        control.visible = titleAvailable || artistAvailable
        _songLabel.text = titleAvailable ? mprisManager.metadata[titleTag].toString() : ""
        _artistLabel.text = artistAvailable ? mprisManager.metadata[artistTag].toString() : ""
        artImage.source = (artUrlTag in mprisManager.metadata) ? mprisManager.metadata[artUrlTag].toString() : ""
    }

    Rectangle {
        anchors.fill: parent
        color: "white"
        radius: LingmoUI.Theme.bigRadius
        opacity: LingmoUI.Theme.darkMode ? 0.2 : 0.7
    }

    RowLayout {
        id: _mainLayout
        anchors.fill: parent
        anchors.margins: LingmoUI.Units.largeSpacing
        spacing: LingmoUI.Units.largeSpacing

        Image {
            id: defaultImage
            width: _mainLayout.height
            height: width
            source: "qrc:/images/media-cover.svg"
            sourceSize: Qt.size(width, height)
            visible: !artImage.visible

            layer.enabled: true
            layer.effect: OpacityMask {
                maskSource: Item {
                    width: defaultImage.width
                    height: defaultImage.height

                    Rectangle {
                        anchors.fill: parent
                        radius: LingmoUI.Theme.bigRadius
                    }
                }
            }
        }

        Image {
            id: artImage
            Layout.preferredHeight: _mainLayout.height
            Layout.preferredWidth: _mainLayout.height
            visible: status === Image.Ready
            // sourceSize: Qt.size(width, height)
            fillMode: Image.PreserveAspectFit

            layer.enabled: true
            layer.effect: OpacityMask {
                maskSource: Item {
                    width: artImage.width
                    height: artImage.height

                    Rectangle {
                        anchors.fill: parent
                        radius: LingmoUI.Theme.bigRadius
                    }
                }
            }
        }

        Item {
            Layout.fillHeight: true
            Layout.fillWidth: true

            ColumnLayout {
                anchors.fill: parent

                Item {
                    Layout.fillHeight: true
                }

                Label {
                    id: _songLabel
                    Layout.fillWidth: true
                    visible: _songLabel.text !== ""
                    elide: Text.ElideRight
                }

                Label {
                    id: _artistLabel
                    Layout.fillWidth: true
                    visible: _artistLabel.text !== ""
                    elide: Text.ElideRight
                }

                Item {
                    Layout.fillHeight: true
                }
            }
        }

        Item {
            id: _buttons
            Layout.fillHeight: true
            Layout.preferredWidth: _buttonsLayout.implicitWidth

            RowLayout {
                id: _buttonsLayout
                anchors.fill: parent
                spacing: LingmoUI.Units.smallSpacing

                IconButton {
                    width: 30
                    height: 30
                    source: "qrc:/images/" + (LingmoUI.Theme.darkMode ? "dark" : "light") + "/media-skip-backward-symbolic.svg"
                    onLeftButtonClicked: if (mprisManager.canGoPrevious) mprisManager.previous()
                    visible: mprisManager.canGoPrevious
                    Layout.alignment: Qt.AlignRight
                }

                IconButton {
                    width: 30
                    height: 30
                    source: control.isPlaying ? "qrc:/images/" + (LingmoUI.Theme.darkMode ? "dark" : "light") + "/media-playback-pause-symbolic.svg"
                                              : "qrc:/images/" + (LingmoUI.Theme.darkMode ? "dark" : "light") + "/media-playback-start-symbolic.svg"
                    Layout.alignment: Qt.AlignRight
                    visible: mprisManager.canPause || mprisManager.canPlay
                    onLeftButtonClicked:
                        if ((control.isPlaying && mprisManager.canPause) || (!control.isPlaying && mprisManager.canPlay)) {
                            mprisManager.playPause()
                        }
                }

                IconButton {
                    width: 30
                    height: 30
                    source: "qrc:/images/" + (LingmoUI.Theme.darkMode ? "dark" : "light") + "/media-skip-forward-symbolic.svg"
                    Layout.alignment: Qt.AlignRight
                    visible: mprisManager.canGoNext
                    onLeftButtonClicked: if (mprisManager.canGoNext) mprisManager.next()
                }
            }
        }
    }
}
