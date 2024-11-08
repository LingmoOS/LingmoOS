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

import QtQuick 2.12
import QtQuick.Layouts 1.12

import org.lingmo.quick.items 1.0 as LingmoUIItems
import org.lingmo.quick.platform 1.0 as Platform

MouseArea {
    id: root
    property alias icon: appIcon
    property alias text: appName
    property alias background: styleBackground
    property bool isRecentInstalled: false

    hoverEnabled: true

    LingmoUIItems.Tooltip {
        anchors.fill: parent
        mainText: text.text
        posFollowCursor: true
        active: text.truncated
    }

    LingmoUIItems.StyleBackground {
        id: styleBackground
        anchors.fill: parent
        radius: Platform.Theme.normalRadius
        useStyleTransparency: false
        paletteRole: Platform.Theme.WindowText
        alpha: root.containsPress ? 0.15 : root.containsMouse ? 0.08 : 0

        ColumnLayout {
            anchors.fill: parent
            anchors.topMargin: 16
            anchors.bottomMargin: 16
            spacing: 8

            LingmoUIItems.Icon {
                id: appIcon
                Layout.preferredWidth: styleBackground.width * 0.6
                Layout.preferredHeight: width
                Layout.alignment: Qt.AlignHCenter
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.preferredHeight: appName.contentHeight
                Layout.maximumHeight: appName.contentHeight
                Layout.alignment: Qt.AlignHCenter

                LingmoUIItems.StyleBackground {
                    id: tagPoint
                    property int tagSize: isRecentInstalled ? 8 : 0
                    Layout.alignment: Qt.AlignVCenter
                    visible: isRecentInstalled
                    Layout.preferredHeight: tagSize
                    Layout.preferredWidth: tagSize
                    radius: width / 2
                    useStyleTransparency: false
                    paletteRole: Platform.Theme.Highlight
                }

                LingmoUIItems.StyleText {
                    id: appName
                    Layout.maximumWidth: styleBackground.width - tagPoint.width
                    Layout.preferredHeight: contentHeight
                    Layout.maximumHeight: contentHeight

                    elide: Text.ElideRight
                    paletteRole: Platform.Theme.Text
                    horizontalAlignment: Text.AlignHCenter
                }
            }
        }
    }
}
