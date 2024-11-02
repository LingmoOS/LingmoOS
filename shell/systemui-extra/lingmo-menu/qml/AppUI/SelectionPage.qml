/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

import QtQuick 2.12
import QtQuick.Layouts 1.2
import org.lingmo.menu.core 1.0
import org.lingmo.quick.platform 1.0 as Platform
import org.lingmo.quick.items 1.0 as LingmoUIItems

Item {
    id: root
    signal viewHideFinished()
    signal labelSelected(string labelId)

    function viewShowStart() {
        viewShow.start();
    }

    function viewFocusEnable() {
        selectionArea.focus = true;
    }

    Component.onCompleted: mainWindow.visibleChanged.connect(viewHideFinished)

    ParallelAnimation {
        id: viewShow
        NumberAnimation { target: selectionArea; property: "scale"; easing.type: Easing.InOutCubic; from: 1.5; to: 1.0; duration: 300}
        NumberAnimation { target: selectionArea; property: "opacity"; easing.type: Easing.InOutCubic; from: 0; to: 1.0; duration: 300}
        onFinished: {
            viewFocusEnable();
        }
    }

    ParallelAnimation {
        id: viewHide
        NumberAnimation { target: selectionArea; property: "scale"; easing.type: Easing.InOutCubic; from: 1.0; to: 1.5 ;duration: 300}
        NumberAnimation { target: selectionArea; property: "opacity"; easing.type: Easing.InOutCubic; from: 1.0; to: 0 ;duration: 300}
        onFinished: {
            selectionArea.focus = false;
            viewHideFinished();
        }
    }

    MouseArea {
        anchors.fill: parent
        onClicked: viewHide.start();
    }

    GridView {
        id: selectionArea
        anchors.centerIn: parent
        interactive: false
        property int itemWidth: 0
        property int itemHeight: 40
        property int column: width / itemWidth
        cellWidth: itemWidth; cellHeight: itemHeight

        onCountChanged: {
            if (count === 0) {
                viewHide.start();
            }
        }

        function itemAtIndex(index) {
            if (index < 0 || index > count - 1) {
                return;
            } else {
                for (var i = 0; i <= count; i++) {
                    var item;
                    item = selectionArea.children[0].children[i];
                    if (item.index === index) {
                        return item;
                    }
                }
            }
        }

        function skipDisableItem() {
            while (itemAtIndex(currentIndex).isDisable) {
                if (currentIndex === count - 1) {
                    currentIndex = 0;
                } else {
                    currentIndex++;
                }
            }
        }

        onActiveFocusChanged: {
            if (activeFocus) {
                currentIndex = 0;
                skipDisableItem();
            }
        }

        Keys.onRightPressed: {
            if(currentIndex === count - 1) {
                currentIndex = 0;
            } else {
                currentIndex++;
            }
            skipDisableItem();
        }
        Keys.onLeftPressed: {
            if(currentIndex === 0) {
                currentIndex = count - 1;
            } else {
                currentIndex--;
            }

            while (itemAtIndex(currentIndex).isDisable) {
                if (currentIndex === 0) {
                    currentIndex = count - 1;
                } else {
                    currentIndex--;
                }
            }
        }
        Keys.onDownPressed: {
            if(currentIndex > count - 1 - column) {
                return;
            } else {
                currentIndex = currentIndex + column;
            }

            while (itemAtIndex(currentIndex).isDisable) {
                if (currentIndex > count - 1 - column) {
                    currentIndex = currentIndex - column;
                } else {
                    currentIndex = currentIndex + column;
                }
            }
        }
        Keys.onUpPressed: {
            if(currentIndex < column) {
                return;
            } else {
                currentIndex = currentIndex - column;
            }

            while (itemAtIndex(currentIndex).isDisable) {
                if (currentIndex < column) {
                    currentIndex = currentIndex + column;
                } else {
                    currentIndex = currentIndex - column;
                }
            }
        }

        state: count < 20 ? "functionArea" : "AlphabetArea"
        states: [
            State {
                name: "functionArea"
                PropertyChanges { target: selectionArea; itemWidth: 80; width: itemWidth * 2; height: itemHeight * 7 }
            },
            State {
                name: "AlphabetArea"
                PropertyChanges { target: selectionArea; itemWidth: 40; width: itemWidth * 5; height: itemHeight * 6 }
            }
        ]

        model: modelManager.getLabelModel()
        delegate: LingmoUIItems.StyleBackground {
            id: labelItem
            height: selectionArea.itemHeight; width: selectionArea.itemWidth
            property int index: model.index
            property bool isDisable: model.isDisable
            property string displayName: model.displayName
            property string id: model.id
            alpha: (itemMouseArea.containsPress && !model.isDisable) ? 0.82 : (itemMouseArea.containsMouse && !model.isDisable) ? 0.55 : 0.00
            useStyleTransparency: false
            radius: 8
            focus: true
            Keys.onPressed: {
                if (event.key === Qt.Key_Enter || event.key === Qt.Key_Return) {
                    viewHide.start();
                    root.labelSelected(selectionArea.itemAtIndex(selectionArea.currentIndex).id)
                }
            }
            states: State {
                when: labelItem.activeFocus
                PropertyChanges {
                    target: labelItem
                    borderColor: Platform.Theme.Highlight
                    border.width: 2
                }
            }
            LingmoUIItems.StyleText  {
                anchors.fill: parent
                text: model.displayName
                alpha: model.isDisable ? 0.2 : 0.9
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
            }

            MouseArea {
                id: itemMouseArea
                anchors.fill: parent
                hoverEnabled: !model.isDisable
                onClicked: {
                    if (!model.isDisable) {
                        viewHide.start();
                        root.labelSelected(model.id);
                    }
                }
                onEntered: {
                    selectionArea.focus = false;
                }
            }
        }
    }
}
