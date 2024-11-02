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
 * Authors: hxf <hewenfei@kylinos.cn>
 *
 */

import QtQuick 2.15
import QtQuick.Layouts 1.15

import org.lingmo.quick.widgets 1.0
import org.lingmo.quick.container 1.0
import org.lingmo.quick.items 1.0
import org.lingmo.quick.platform 1.0

WidgetContainerItem {
    id: containerItem
    property int orientation: WidgetContainer.orientation.Horizontal
    property bool isHorizontal: true


    onParentChanged: {
        anchors.fill = parent
    }

    StyleBackground {
        anchors.fill: parent
        borderColor: Theme.ButtonText
        borderAlpha: 0.25
        border.width: 1


        GridLayout {
            id: mainLayout
            z: 10
            anchors.fill: parent
            rows:  1
            columns: repeater.count
            rowSpacing: 4
            columnSpacing: 4

            Repeater {
                id: repeater
                model: containerItem.widgetItemModel
                delegate: widgetLoaderComponent
            }
        }

        Component {
            id: widgetLoaderComponent

            Item {
                id: widgetParent
                clip: true

                property int index: model.index
                property Item widgetItem: model.widgetItem

                function computeFillWidth(w) {
                    return w < 0 ? mainLayout.width : Math.min(w, mainLayout.width);
                }

                function computeFillHeight(h) {
                    return h < 0 ? mainLayout.height : Math.min(h, mainLayout.height);
                }

                Layout.fillWidth: widgetItem && widgetItem.Layout.fillWidth
                Layout.fillHeight: widgetItem && widgetItem.Layout.fillHeight

                Layout.minimumWidth: widgetItem ? computeFillWidth(widgetItem.Layout.minimumWidth) : mainLayout.width
                Layout.minimumHeight: widgetItem ? computeFillHeight(widgetItem.Layout.minimumHeight) : mainLayout.height

                Layout.maximumWidth: widgetItem ? computeFillWidth(widgetItem.Layout.maximumWidth) : mainLayout.width
                Layout.maximumHeight: widgetItem ? computeFillHeight(widgetItem.Layout.maximumHeight) : mainLayout.height

                Layout.preferredWidth: widgetItem ? computeFillWidth(widgetItem.Layout.preferredWidth) : mainLayout.width
                Layout.preferredHeight: widgetItem ? computeFillHeight(widgetItem.Layout.preferredHeight) : mainLayout.height

                onWidgetItemChanged: {
                    if (widgetItem) {
                        widgetItem.parent = widgetParent
                        widgetItem.anchors.fill = widgetParent
                    }
                }
            }
        }
    }
}
