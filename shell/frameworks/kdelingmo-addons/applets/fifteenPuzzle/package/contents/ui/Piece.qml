/*
 * SPDX-FileCopyrightText: 2014 Jeremy Whiting <jpwhiting@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.15
import QtQuick.Layouts 1.15

import org.kde.lingmo.core as LingmoCore
import org.kde.lingmoui as LingmoUI
import org.kde.lingmo.components 3.0 as LingmoComponents3
import org.kde.lingmo.extras 2.0 as LingmoExtras
import org.kde.lingmo.plasmoid 2.0

Rectangle {
    id: piece
    color: Plasmoid.configuration.boardColor
    border.color: "black"
    border.width: 1
    radius: LingmoUI.Units.cornerRadius
    visible: !empty

    Layout.minimumWidth: 10
    Layout.preferredWidth: 30

    Layout.minimumHeight: 10
    Layout.preferredHeight: 30

    x: boardColumn * (width + margin) + margin / 2
    y: boardRow * (height + margin) + margin / 2
    width: pieceWidth
    height: pieceHeight

    signal activated(int position)

    readonly property int boardSize: Plasmoid.configuration.boardSize
    readonly property int margin: LingmoUI.Units.smallSpacing
    readonly property int pieceWidth: (parent.width - (margin * boardSize)) / boardSize
    readonly property int pieceHeight: (parent.height - (margin * boardSize)) / boardSize
    readonly property int boardColumn: (position % boardSize)
    readonly property int boardRow: Math.floor(position / boardSize)
    readonly property bool empty: number === 0

    property int number
    property int position

    Keys.onPressed: {
        switch (event.key) {
        case Qt.Key_Space:
        case Qt.Key_Enter:
        case Qt.Key_Return:
        case Qt.Key_Select:
            piece.trigger();
            break;
        }
    }
    Accessible.name: pieceNumeral.text
    Accessible.role: Accessible.Button

    function trigger() {
        piece.forceActiveFocus();
        piece.activated(position);
    }

    Behavior on x {
        NumberAnimation {
            duration: LingmoUI.Units.longDuration
            easing.type: Easing.InOutQuad
        }
    }
    Behavior on y {
        NumberAnimation {
            duration: LingmoUI.Units.longDuration
            easing.type: Easing.InOutQuad
        }
    }

    TapHandler {
        onTapped: piece.trigger()
    }

    Loader {
        anchors.fill: parent
        active: parent.activeFocus
        asynchronous: true
        z: 0

        sourceComponent: LingmoExtras.Highlight {
            hovered: true
        }
    }

    LingmoComponents3.Label {
        id: pieceNumeral
        anchors.centerIn: parent
        text: piece.number
        textFormat: Text.PlainText
        color: Plasmoid.configuration.numberColor
        visible: Plasmoid.configuration.showNumerals
        z: 1
    }

    Loader {
        anchors.fill: parent

        active: Plasmoid.configuration.useImage
        asynchronous: true
        z: 0

        sourceComponent: Image {
            id: pieceImage
            source: "image://fifteenpuzzle/" + boardSize + "-" + number + "-" + pieceWidth + "-" + pieceHeight + "-" + Plasmoid.configuration.imagePath
            cache: false
        }
    }
}
