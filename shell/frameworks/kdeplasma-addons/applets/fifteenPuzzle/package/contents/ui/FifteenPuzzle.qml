/*
 * SPDX-FileCopyrightText: 2014 Jeremy Whiting <jpwhiting@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.15
import QtQuick.Layouts 1.15

import org.kde.coreaddons 1.0 as KCoreAddons
import org.kde.plasma.core as PlasmaCore
import org.kde.kirigami 2.20 as Kirigami
import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.plasma.plasmoid 2.0

Item {
    id: main

    Layout.preferredWidth: Math.max(boardSize * 50, controlsRow.width)
    Layout.preferredHeight: boardSize * 50 + controlsRow.height

    readonly property int boardSize: Plasmoid.configuration.boardSize
    property Component piece: Piece {}
    property var pieces: []
    property int currentPosition: -1

    property int seconds: 0

    Keys.onPressed: {
        let newPosition = main.currentPosition;
        switch (event.key) {
        case Qt.Key_Up: {
            if (main.currentPosition < 0) {
                newPosition = (main.boardSize - 1) * main.boardSize;  // Start from bottom
            } else if (main.currentPosition >= main.boardSize) {
                newPosition = main.currentPosition - main.boardSize;
            }
            if (pieces[newPosition].empty) {
                if (main.currentPosition < 0) {
                    newPosition = (main.boardSize - 2) * main.boardSize;
                } else if (newPosition >= main.boardSize) {
                    newPosition -= main.boardSize;
                }
            }
            break;
        }
        case Qt.Key_Down: {
            if (main.currentPosition < 0) {
                newPosition = 0;  // Start from top
            } else if (main.currentPosition < main.boardSize * (main.boardSize - 1)) {
                newPosition = main.currentPosition + main.boardSize;
            }
            if (pieces[newPosition].empty) {
                if (main.currentPosition < 0) {
                    newPosition = main.boardSize;
                } else if (newPosition < main.boardSize * (main.boardSize - 1)) {
                    newPosition += main.boardSize;
                }
            }
            break;
        }
        case Qt.Key_Left: {
            if (main.currentPosition < 0) {
                newPosition = main.boardSize - 1;  // Start from right
            } else if (main.currentPosition % main.boardSize) {
                newPosition = main.currentPosition - 1;
            }
            if (pieces[newPosition].empty) {
                if (main.currentPosition < 0) {
                    newPosition = main.boardSize - 2;
                } else if (newPosition % main.boardSize) {
                    newPosition -= 1;
                }
            }
            break;
        }
        case Qt.Key_Right: {
            if (main.currentPosition < 0) {
                newPosition = 0;  // Start from left
            } else if ((main.currentPosition + 1) % main.boardSize) {
                newPosition = main.currentPosition + 1;
            }
            if (pieces[newPosition].empty) {
                if (main.currentPosition < 0) {
                    newPosition = 1;
                } else if ((newPosition + 1) % main.boardSize) {
                    newPosition += 1;
                }
            }
            break;
        }
        default:
            return;
        }

        // Edge empty case: don't move
        if (pieces[newPosition].empty) {
            newPosition = main.currentPosition;
        }

        pieces[newPosition].forceActiveFocus();
        event.accepted = true;
    }

    function fillBoard() {
        // Clear out old board
        for (const piece of pieces) {
            piece.destroy();
        }
        main.currentPosition = -1;

        pieces = [];
        const count = boardSize * boardSize;
        if (piece.status === Component.Ready) {
            for (let i = 0; i < count; ++i) {
                const newPiece = piece.createObject(mainGrid, {"number": i, "position": i });
                pieces[i] = newPiece;
                newPiece.activeFocusChanged.connect(() => {
                    if (newPiece.activeFocus) {
                        main.currentPosition = newPiece.position;
                    }
                });
                newPiece.activated.connect(pieceClicked);
            }
            shuffleBoard();
        }
    }

    function shuffleBoard() {
        // Hide the solved rectangle in case it was visible
        solvedRect.visible = false;
        main.seconds = 0;
        main.currentPosition = -1;

        const count = boardSize * boardSize;
        for (let i = count - 1; i >= 0; --i) {
            // choose a random number such that 0 <= rand <= i
            const rand = Math.floor(Math.random() * 10) % (i + 1);
            swapPieces(i, rand);
        }

        // make sure the new board is solveable

        // count the number of inversions
        // an inversion is a pair of tiles at positions a, b where
        // a < b but value(a) > value(b)

        // also count the number of lines the blank tile is from the bottom
        let inversions = 0;
        let blankRow = -1;
        for (let i = 0; i < count; ++i) {
            if (pieces[i].empty) {
                blankRow = Math.floor(i / boardSize);
                continue;
            }
            for (let j = 0; j < i; ++j) {
                if (pieces[j].empty) {
                    continue;
                }
                if (pieces[i].number < pieces[j].number) {
                    ++inversions;
                }
            }
        }

        if (blankRow === -1) {
            console.log("Unable to find row of blank tile");
        }

        // we have a solveable board if:
        // size is odd:  there are an even number of inversions
        // size is even: the number of inversions is odd if and only if
        //               the blank tile is on an odd row from the bottom-
        const sizeMod2 = Math.floor(boardSize % 2);
        const inversionsMod2 = Math.floor(inversions % 2);
        const solveable = (sizeMod2 === 1 && inversionsMod2 === 0) ||
                          (sizeMod2 === 0 && (inversionsMod2 === 0) === (Math.floor((boardSize - blankRow) % 2) === 1));
        if (!solveable) {
            // make the grid solveable by swapping two adjacent pieces around
            let pieceA = 0;
            let pieceB = 1;
            if (pieces[pieceA].empty) {
                pieceA = boardSize + 1;
            } else if (pieces[pieceB].empty) {
                pieceB = boardSize;
            }
            swapPieces(pieceA, pieceB);
        }
        secondsTimer.stop();
    }

    // recursive function: performs swap and returns true when it has found an
    // empty piece in the direction given by deltas.
    function swapWithEmptyPiece(position, deltaRow, deltaColumn): bool {
        const row = Math.floor(position / boardSize);
        const column = position % boardSize;

        const nextRow = row + deltaRow;
        const nextColumn = column + deltaColumn;
        const nextPosition = nextRow * boardSize + nextColumn;

        if (nextRow < 0 || nextRow >= boardSize || nextColumn < 0 || nextColumn >= boardSize) {
            return false;
        }

        if (pieces[nextPosition].empty || swapWithEmptyPiece(nextPosition, deltaRow, deltaColumn)) {
            swapPieces(position, nextPosition);
            return true;
        }

        return false;
    }

    function pieceClicked(position) {
        // deltas: up, down, left, right
        for (const [row, col] of [[-1, 0], [1, 0], [0, -1], [0, 1]]) {
            // stop at first direction that has (or rather "had" at this point) the empty piece
            if (swapWithEmptyPiece(position, row, col)) {
                main.currentPosition += col + main.boardSize * row;
                break;
            }
        }
        secondsTimer.start();
        checkSolved();
    }

    function checkSolved() {
        const count = boardSize * boardSize;
        for (let i = 0; i < count - 2; ++i) {
            if (pieces[i].number > pieces[i + 1].number) {
                // Not solved.
                return;
            }
        }
        solved();
    }

    function solved() {
        // Show a message that it was solved.
        console.log("Puzzle was solved");
        solvedRect.visible = true;
        // Stop the timer
        secondsTimer.stop();
    }

    function swapPieces(first, second) {
        const firstPiece = pieces[first];
        const secondPiece = pieces[second];
        let temp = firstPiece.position;
        firstPiece.position = secondPiece.position;
        secondPiece.position = temp;
        temp = pieces[first];
        pieces[first] = pieces[second];
        pieces[second] = temp;
    }

    function timerText() {
        return i18nc("The time since the puzzle started, in minutes and seconds",
                     "Time: %1", KCoreAddons.Format.formatDuration(seconds * 1000, KCoreAddons.FormatTypes.FoldHours));
    }

    Rectangle {
        id: mainGrid
        color: Kirigami.Theme.backgroundColor
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            bottom: controlsRow.top
            bottomMargin: Kirigami.Units.smallSpacing
        }

        activeFocusOnTab: true

        onActiveFocusChanged: {
            // Move focus to the first non-empty piece
            if (activeFocus) {
                if (main.currentPosition < 0) {
                    if (main.pieces[0].empty) {
                        main.pieces[1].forceActiveFocus();
                    } else {
                        main.pieces[0].forceActiveFocus();
                    }
                } else {
                    main.pieces[main.currentPosition].forceActiveFocus();
                }
            }
        }
    }

    RowLayout {
        id: controlsRow
        anchors {
            margins: Kirigami.Units.smallSpacing
            bottom: parent.bottom
            horizontalCenter: parent.horizontalCenter
        }
        PlasmaComponents3.Button {
            id: button
            Layout.fillWidth: true
            icon.name: "roll"
            text: i18nc("@action:button", "Shuffle");
            onClicked: main.shuffleBoard();
        }

        PlasmaComponents3.Label {
            id: timeLabel
            Layout.fillWidth: true
            text: main.timerText()
            textFormat: Text.PlainText
            color: Kirigami.Theme.textColor
        }
    }

    Rectangle {
        id: solvedRect
        visible: false
        anchors.fill: mainGrid
        color: Kirigami.Theme.backgroundColor
        z: 0

        Image {
            id: solvedImage
            anchors.fill: parent
            z: 1
            source: "image://fifteenpuzzle/" + boardSize + "-all-0-0-" + Plasmoid.configuration.imagePath
            visible: Plasmoid.configuration.useImage
            cache: false
            function update() {
                const tmp = source;
                source = "";
                source = tmp;
            }
        }

        PlasmaComponents3.Label {
            id: solvedLabel
            anchors.centerIn: parent
            color: Kirigami.Theme.textColor
            text: i18nc("@info", "Solved! Try again.")
            textFormat: Text.PlainText
            z: 2
        }
    }

    Timer {
        id: secondsTimer
        interval: 1000
        repeat: true

        onTriggered: ++main.seconds
    }

    Connections {
        target: Plasmoid.configuration
        function onBoardSizeChanged() {
            main.fillBoard();
            solvedImage.update();
        }
    }

    Connections {
        target: Plasmoid.configuration
        function onImagePathChanged() {
            main.fillBoard();
            solvedImage.update();
        }
    }

    Component.onCompleted: {
        main.fillBoard();
        solvedImage.update();
    }
}
