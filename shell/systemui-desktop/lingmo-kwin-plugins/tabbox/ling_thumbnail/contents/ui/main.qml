import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.kquickcontrolsaddons 2.0
import org.kde.kwin 2.0 as KWin

import LingmoUI 1.0 as LingmoUI

// https://techbase.kde.org/Development/Tutorials/KWin/WindowSwitcher

KWin.Switcher {
    id: tabBox
    currentIndex: thumbnailGridView.currentIndex

    Window {
        id: dialog
        visible: tabBox.visible
        flags: Qt.BypassWindowManagerHint | Qt.FramelessWindowHint
        color: "transparent"

        property int maxWidth: tabBox.screenGeometry.width * 0.95
        property int maxHeight: tabBox.screenGeometry.height * 0.7
        property int optimalWidth: thumbnailGridView.cellWidth * gridColumns
        property int optimalHeight: thumbnailGridView.cellHeight * gridRows
        property int maxGridColumnsByWidth: Math.floor(maxWidth / thumbnailGridView.cellWidth)
        property int gridColumns: maxGridColumnsByWidth
        property int gridRows: Math.ceil(thumbnailGridView.count / gridColumns)

        width: Math.min(Math.max(thumbnailGridView.cellWidth, optimalWidth), maxWidth)
        height: Math.min(Math.max(thumbnailGridView.cellHeight, optimalHeight), maxHeight)

        x: tabBox.screenGeometry.x + (tabBox.screenGeometry.width - dialog.width) / 2
        y: tabBox.screenGeometry.y + (tabBox.screenGeometry.height - dialog.height) / 2

        LingmoUI.WindowHelper {
            id: windowHelper
        }

        LingmoUI.WindowBlur {
            view: dialog
            geometry: Qt.rect(dialog.x, dialog.y, dialog.width, dialog.height)
            windowRadius: _background.radius
            enabled: windowHelper.compositing
        }

        LingmoUI.WindowShadow {
            view: dialog
            geometry: Qt.rect(dialog.x, dialog.y, dialog.width, dialog.height)
            radius: _background.radius
        }

        Rectangle {
            id: _background
            anchors.fill: parent
            radius: windowHelper.compositing ? 14 : 0
            color: LingmoUI.Theme.backgroundColor
            opacity: windowHelper.compositing ? LingmoUI.Theme.darkMode ? 0.3 : 0.4 : 1.0

            border.color: LingmoUI.Theme.darkMode ? "#686868" : "#D9D9D9"
            border.width: windowHelper.compositing ? 0 : 1
        }

        onVisibleChanged: {
            if (visible) {
                dialogMainItem.calculateColumnCount();
            } else {
                thumbnailGridView.highCount = 0;
            }
        }

        Item {
            id: dialogMainItem
            anchors.fill: parent

            property real screenFactor: tabBox.screenGeometry.width / tabBox.screenGeometry.height

            property bool canStretchX: false
            property bool canStretchY: false

            clip: true

            // simple greedy algorithm
            function calculateColumnCount() {
                // respect screenGeometry
                var c = Math.min(thumbnailGridView.count, dialog.maxGridColumnsByWidth);

                var residue = thumbnailGridView.count % c;
                if (residue == 0) {
                    dialog.gridColumns = c;
                    return;
                }

                // start greedy recursion
                dialog.gridColumns = columnCountRecursion(c, c, c - residue);
            }

            // step for greedy algorithm
            function columnCountRecursion(prevC, prevBestC, prevDiff) {
                var c = prevC - 1;

                // don't increase vertical extent more than horizontal
                // and don't exceed maxHeight
                if (prevC * prevC <= thumbnailGridView.count + prevDiff ||
                        dialog.maxHeight < Math.ceil(thumbnailGridView.count / c) * thumbnailGridView.cellHeight) {
                    return prevBestC;
                }
                var residue = thumbnailGridView.count % c;
                // halts algorithm at some point
                if (residue == 0) {
                    return c;
                }
                // empty slots
                var diff = c - residue;

                // compare it to previous count of empty slots
                if (diff < prevDiff) {
                    return columnCountRecursion(c, c, diff);
                } else if (diff == prevDiff) {
                    // when it's the same try again, we'll stop early enough thanks to the landscape mode condition
                    return columnCountRecursion(c, prevBestC, diff);
                }
                // when we've found a local minimum choose this one (greedy)
                return columnCountRecursion(c, prevBestC, diff);
            }

            property bool mouseEnabled: false
            MouseArea {
                id: mouseDetector
                anchors.fill: parent
                hoverEnabled: true
                onPositionChanged: dialogMainItem.mouseEnabled = true
            }

            GridView {
                id: thumbnailGridView
                model: tabBox.model
                // interactive: false // Disable drag to scroll

                anchors.fill: parent

                property int captionRowHeight: 22
                property int thumbnailWidth: 300
                property int thumbnailHeight: thumbnailWidth * (1.0 / dialogMainItem.screenFactor)
                cellWidth: thumbnailWidth
                cellHeight: captionRowHeight + thumbnailHeight
                height: cellHeight

                clip: true

                // allow expansion on increasing count
                property int highCount: 0
                onCountChanged: {
                    if (highCount != count) {
                        dialogMainItem.calculateColumnCount();
                        highCount = count;
                    }
                }

                delegate: Item {
                    property bool isCurrent: thumbnailGridView.currentIndex === index

                    width: thumbnailGridView.cellWidth
                    height: thumbnailGridView.cellHeight

                    MouseArea {
                        anchors.fill: parent
                        // hoverEnabled: dialogMainItem.mouseEnabled
                        // onEntered: parent.hover()
                        onClicked: {
                            parent.select()
                            // dialog.close() // Doesn't end the effects until you release Alt.
                        }
                    }
                    function select() {
                        thumbnailGridView.currentIndex = index;
                        thumbnailGridView.currentIndexChanged(thumbnailGridView.currentIndex);
                    }

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 16

                        QIconItem {
                            id: iconItem
                            // source: model.icon
                            icon: model.icon
                            width: parent.height * 0.5
                            height: parent.height * 0.5
                            state: index == thumbnailGridView.currentIndex ? QIconItem.ActiveState : QIconItem.DefaultState
                            Layout.alignment: Qt.AlignHCenter
                        }

                        Label {
                            text: model.caption
                            Layout.fillWidth: true
                            elide: Text.ElideRight
                            horizontalAlignment: Text.AlignHCenter
                            color: isCurrent ? LingmoUI.Theme.highlightedTextColor : LingmoUI.Theme.textColor
                        }
                    }
                } // GridView.delegate

                highlight: Item {
                    id: highlightItem

                    Rectangle {
                        anchors.fill: parent
                        anchors.margins: LingmoUI.Units.largeSpacing
                        radius: _background.radius
                        color: LingmoUI.Theme.highlightColor
                        opacity: 0.7
                    }
                }

                Connections {
                    target: tabBox
                    function onCurrentIndexChanged() {
                        thumbnailGridView.currentIndex = tabBox.currentIndex
                    }
                }
            } // GridView

            // This doesn't work, nor does keyboard input work on any other tabbox skin (KDE 5.7.4)
            // It does work in the preview however.
            Keys.onPressed: {
                console.log('keyPressed', event.key)
                if (event.key == Qt.Key_Left) {
                    thumbnailGridView.moveCurrentIndexLeft();
                } else if (event.key == Qt.Key_Right) {
                    thumbnailGridView.moveCurrentIndexRight();
                } else if (event.key == Qt.Key_Up) {
                    thumbnailGridView.moveCurrentIndexUp();
                } else if (event.key == Qt.Key_Down) {
                    thumbnailGridView.moveCurrentIndexDown();
                } else {
                    return;
                }

                thumbnailGridView.currentIndexChanged(thumbnailGridView.currentIndex);
            }
        } // Dialog.mainItem
    } // Dialog
}
