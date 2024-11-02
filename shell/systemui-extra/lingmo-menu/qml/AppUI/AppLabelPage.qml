/*
 * Copyright (C) 2024, KylinSoft Co., Ltd.
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

import org.lingmo.menu.core 1.0

import org.lingmo.quick.items 1.0 as LingmoUIItems
import org.lingmo.quick.platform 1.0 as Platform

GridView {
    property var labelBottle: null
    property int labelColum: labelBottle === null ? 0 : labelBottle.column
    property int labelRow: Math.ceil(count / labelColum)

    signal labelSelected(string label)

    // 默认为小屏幕下尺寸
    width: 200
    height: childrenRect.height
    cellWidth: width / labelColum
    cellHeight: 40

    model: labelBottle === null ? [] : labelBottle.labels
    delegate: MouseArea {
        width: GridView.view.cellWidth
        height: GridView.view.cellHeight
        focus: true
        hoverEnabled: true

        LingmoUIItems.Tooltip {
            anchors.fill: parent
            mainText: modelData.label
            posFollowCursor: true
            active: labelArea.showTooltip
        }

        onClicked: {
            GridView.view.labelSelected(modelData.label);
            GridView.view.currentIndex = model.index
        }

        Keys.onPressed: (event) => {
            if (event.key === Qt.Key_Enter || event.key === Qt.Key_Return) {
                GridView.view.labelSelected(modelData.label);
            }
        }

        LingmoUIItems.StyleBackground {
            id: labelArea
            property bool showTooltip: labelText.visible ? labelText.truncated : false

            anchors.fill: parent

            radius: Platform.Theme.normalRadius
            useStyleTransparency: false
            paletteRole: Platform.Theme.WindowText
            alpha: parent.containsPress ? 0.15 : parent.containsMouse ? 0.08 : 0.0

            border.width: (parent.GridView.isCurrentItem && !mainWindow.isFullScreen) ? 2 : 0
            borderColor: Platform.Theme.Highlight

            LingmoUIItems.StyleText {
                id: labelText
                anchors.fill: parent
                visible: modelData.type === LabelItem.Text
                text: modelData.display
                elide: Text.ElideRight
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
            }

            LingmoUIItems.Icon {
                anchors.centerIn: parent
                width: parent.height/2
                height: parent.height/2
                visible: modelData.type === LabelItem.Icon
                source: modelData.type === LabelItem.Icon ? modelData.display : ""
                mode: LingmoUIItems.Icon.AutoHighlight
            }
        }
    }

    Keys.onRightPressed: {
        if(currentIndex === count - 1) {
            currentIndex = 0;
        } else {
            currentIndex++;
        }
    }
    Keys.onLeftPressed: {
        if(currentIndex === 0) {
            currentIndex = count - 1;
        } else {
            currentIndex--;
        }
    }
    Keys.onDownPressed: {
        if(Math.floor(currentIndex / labelColum) < labelRow - 1) {
            currentIndex = currentIndex + labelColum;
        }
    }
    Keys.onUpPressed: {
        if(Math.floor(currentIndex / labelColum) > 0) {
            currentIndex = currentIndex - labelColum;
        }
    }
}
