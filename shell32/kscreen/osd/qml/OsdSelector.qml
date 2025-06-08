/*
    SPDX-FileCopyrightText: 2017 Daniel Vrátil <dvratil@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.10

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.plasma.components 3.0 as PlasmaComponents

import org.kde.KScreen 1.0


Control {
    id: root
    property string infoText
    property var actions
    signal clicked(int actionId)

    leftPadding: shadow.margins.left + background.margins.left
    rightPadding: shadow.margins.right + background.margins.right
    topPadding: shadow.margins.top + background.margins.top
    bottomPadding: shadow.margins.bottom + background.margins.bottom

    contentItem : ColumnLayout {
        RowLayout {
            Repeater {
                id: actionRepeater
                property int currentIndex: 0
                model: root.actions
                delegate: PlasmaComponents.Button {
                    property int actionId: modelData.action

                    Accessible.name: modelData.label

                    icon.name: modelData.iconName
                    icon.height: PlasmaCore.Units.gridUnit * 8
                    icon.width: PlasmaCore.Units.gridUnit * 8

                    onClicked: root.clicked(actionId)
                    onHoveredChanged: {
                        actionRepeater.currentIndex = index
                    }

                    activeFocusOnTab: true

                    // use checked only indirectly, since its binding will break
                    property bool current: index == actionRepeater.currentIndex
                    onCurrentChanged: {
                        if (current) {
                            checked = true
                            root.infoText = modelData.label
                            forceActiveFocus()
                        } else {
                            checked = false
                        }
                    }
                    onActiveFocusChanged: {
                        if (activeFocus) {
                            actionRepeater.currentIndex = index
                        }
                    }
                }
            }
        }

        PlasmaExtras.Heading {
            text: root.infoText
            horizontalAlignment: Text.AlignHCenter
            maximumLineCount: 2
            wrapMode: Text.WordWrap

            Layout.fillWidth: true
            Layout.margins: Math.floor(PlasmaCore.Units.smallSpacing / 2)
        }

        // Shift current by delta, but do not wrap around when repeat is true.
        function wrappingAdd(count: int, current: int, delta: int, repeat: bool): int {
            const next = current + delta;
            // Rule out invalid states.
            if (count === 0 || current < 0 || current >= count) {
                return current;
            }
            // Don't wrap on autorepeat.
            if (repeat && (next < 0 || next >= count)) {
                return current;
            }
            // Add an extra `count`, so that wrapping % works predictably with positive values only.
            // This assumes that delta is not smaller than `-count` (usually just -1, 0 or +1).
            return (next + count) % count;
        }

        function move(event) {
            actionRepeater.currentIndex = wrappingAdd(actionRepeater.count, actionRepeater.currentIndex,
                (event.key === Qt.Key_Left) ? -1 : 1, event.isAutoRepeat);
        }

        Keys.onPressed: {
            switch (event.key) {
                case Qt.Key_Return:
                case Qt.Key_Enter:
                    clicked(actionRepeater.itemAt(actionRepeater.currentIndex).actionId)
                    break
                case Qt.Key_Right:
                case Qt.Key_Left:
                    move(event)
                    break
                case Qt.Key_Escape:
                    clicked(OsdAction.NoAction)
                    break
            }
        }
    }

      background: PlasmaCore.FrameSvgItem {
        id: shadow
        imagePath: "dialogs/background"
        prefix: "shadow"

        PlasmaCore.FrameSvgItem {
            id: background
            anchors.leftMargin: shadow.margins.left
            anchors.rightMargin: shadow.margins.right
            anchors.topMargin: shadow.margins.top
            anchors.bottomMargin: shadow.margins.bottom
            anchors.fill: parent
            imagePath: "solid/dialogs/background"
        }
    }
}
