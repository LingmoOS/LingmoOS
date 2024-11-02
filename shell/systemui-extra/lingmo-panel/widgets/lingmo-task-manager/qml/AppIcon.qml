/*
 *
 *  Copyright (C) 2024, KylinSoft Co., Ltd.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 *  Authors: qiqi49 <qiqi@kylinos.cn>
 *
 */

import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtGraphicalEffects 1.12
import org.lingmo.quick.items 1.0 as LingmoUIItems
import org.lingmo.quick.platform 1.0 as Platform
import org.lingmo.panel.taskManager 1.0 as LingmoUITaskManager

Item {
    property alias appIcon: icon
    property alias appBackgroud: backgroud
    property bool isOpenGL: true

    function updateIconGeometry() {
        if (taskItemData.CurrentWinIdList) {
            if (taskItemData.CurrentWinIdList.length > 0) {
                Platform.WindowManager.setMinimizedGeometry(taskItemData.CurrentWinIdList, backgroud);
            }
        }
    }

    LingmoUIItems.StyleBackground {
        id: backgroud
        anchors.fill: parent
        anchors.margins: 2
        useStyleTransparency: false
        paletteRole: Platform.Theme.BrightText
        radius: Platform.Theme.normalRadius

        Rectangle {
            id: demandsAttention
            anchors.fill: parent
            color: "#FF9100"
            z: -1
            radius: 8
            visible: taskItemData.DemandsAttentionWinIdList ? (taskItemData.DemandsAttentionWinIdList.length > 0) : false

            NumberAnimation on opacity {
                running: demandsAttention.visible
                loops: 4
                from: 0.25
                to: 0.6
                duration: 450
            }
        }

        LingmoUIItems.Icon {
            id: icon
            anchors.margins: 4
            anchors.fill: parent
            Behavior on scale {
                NumberAnimation { duration: 200 }
            }
        }

        Loader {
            anchors.right: icon.right
            anchors.rightMargin: -3
            anchors.top: icon.top
            anchors.topMargin: -3
            active: taskItemData.UnreadMessagesNum > 0
            sourceComponent: Rectangle {
                id: unreadMessagesBase
                height: unreadMessagesNum.contentHeight
                width: height + additionalWidth
                radius: height / 2
                property int additionalWidth: {
                    if (taskItemData.UnreadMessagesNum < 10) {
                        return 0;
                    } else if (taskItemData.UnreadMessagesNum < 100){
                        return 6;
                    } else {
                        return 12;
                    }
                }
                color: "#F3222D"
                opacity: 0.75
                layer.enabled: isOpenCV
                layer.effect: DropShadow {
                    radius: 2
                    samples: 5
                    color: Platform.Theme.color(Platform.Theme.Shadow, Platform.Theme.Active, 1)
                }
                LingmoUIItems.StyleText {
                    id: unreadMessagesNum
                    anchors.fill: parent
                    horizontalAlignment: Text.AlignHCenter
                    paletteRole: Platform.Theme.Light
                    pointSizeOffset: -2
                    text: {
                        if (taskItemData.UnreadMessagesNum < 100) {
                            return taskItemData.UnreadMessagesNum;
                        } else {
                            return "99+"
                        }
                    }
                }
            }
        }
    }
    LingmoUIItems.StyleBackground {
        height: 4
        width: taskItemData.CurrentWinIdList ? Math.min(taskItemData.CurrentWinIdList.length * 8, 16) : 16
        radius: 2

        anchors.verticalCenter: backgroud.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        useStyleTransparency: false
        paletteRole: taskItemData.HasActiveWindow? Platform.Theme.Highlight : Platform.Theme.BrightText
        alpha: taskItemData.HasActiveWindow? 1 : 0.3
        Behavior on width {
            NumberAnimation { duration: 200 }
        }
    }
}
