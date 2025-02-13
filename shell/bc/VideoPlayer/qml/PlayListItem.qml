/*
 * SPDX-FileCopyrightText: 2020 George Florea Bănuș <georgefb899@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtGraphicalEffects 1.12

import LingmoUI 1.0 as CuteUI

Item {
    id: root

    property bool isPlaying: model.isPlaying
    property string rowNumber: (index + 1).toString()
    property var alpha: 1

    height: label.font.pointSize * 3 + 10
    width: ListView.view.width

    signal doubleClicked()

    Rectangle {
        anchors.fill: parent
        color: CuteUI.Theme.backgroundColor
        opacity: 0.5
    }

    Menu {
        id: _itemMenu

        MenuItem {
            text: qsTr("Play")
            onTriggered: play()
        }

        MenuItem {
            text: qsTr("Remove from list")
            onTriggered: mpv.playlistModel.remove(index)
        }
    }

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton | Qt.RightButton

        onClicked: {
            if (mouse.button === Qt.RightButton)
                _itemMenu.popup()
        }

        onDoubleClicked: {
            root.doubleClicked()
            play()
        }
    }

    RowLayout {
        anchors.fill: parent
        spacing: CuteUI.Units.largeSpacing

        Label {
            text: pad(root.rowNumber, playlistView.count.toString().length)
            visible: true
            font.pointSize: (rootWindow.isFullScreen() && playList.bigFont)
                            ? CuteUI.Units.gridUnit
                            : CuteUI.Units.gridUnit - 6
            horizontalAlignment: Qt.AlignCenter
            Layout.leftMargin: CuteUI.Units.largeSpacing

            function pad(number, length) {
                while (number.length < length)
                    number = "0" + number;
                return number;
            }
        }

        Label {
            id: label

            horizontalAlignment: Qt.AlignLeft
            verticalAlignment: Qt.AlignVCenter
            elide: Text.ElideRight
            font.pointSize: (rootWindow.isFullScreen() && playList.bigFont)
                            ? CuteUI.Units.gridUnit
                            : CuteUI.Units.gridUnit - 6
            font.weight: Font.Normal
            color: isPlaying ? CuteUI.Theme.highlightColor : CuteUI.Theme.textColor
            text: model.name
            layer.enabled: true
            Layout.fillWidth: true
            // Layout.leftMargin: PlaylistSettings.showRowNumber || isPlaying ? 0 : CuteUI.Units.largeSpacing
        }

        Label {
            text: model.duration
            visible: model.duration.length > 0
            font.pointSize: (rootWindow.isFullScreen() && playList.bigFont)
                            ? CuteUI.Units.gridUnit
                            : CuteUI.Units.gridUnit - 6
            horizontalAlignment: Qt.AlignCenter
            Layout.margins: CuteUI.Units.largeSpacing
        }
    }

    function play() {
        mpv.playlistModel.setPlayingVideo(index)
        mpv.loadFile(path)
        mpv.pause = false
    }
}
