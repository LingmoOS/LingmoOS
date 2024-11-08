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
 * Authors: hxf <hewenfei@kylinos.cn>
 *          zy-yuan1 <zhangyuanyuan1@kylinos.cn>
 */

import QtQuick 2.15
import QtQuick.Layouts 1.15

import org.lingmo.quick.widgets 1.0
import org.lingmo.quick.items 1.0
import org.lingmo.panel.calendar 1.0
import org.lingmo.quick.platform 1.0

WidgetItem {
    id: root
    property bool isHorizontal: Widget.orientation == Types.Horizontal

    Layout.fillWidth: !isHorizontal
    Layout.fillHeight: isHorizontal
    Layout.preferredWidth: isHorizontal ? backGround.width + 8 + iconBackGround.width : width
    Layout.preferredHeight: isHorizontal ? height : backGround.height + 8 + iconBackGround.height

    onWidthChanged: {
        if (!isHorizontal) {
            updateText();
        }
    }

    onHeightChanged: {
        if (isHorizontal) {
            updateText();
        }
    }

    Component.onCompleted: {
        updateText();
        //isClearRedpoint();
    }

    Calendar {
        id: calendar

        onTimeUpdated: {
            updateText();
        }
    }

    function updateText() {
        let text = calendar.time;

        //time ap
        let s = text;
        if (isOverFlow(s)) {
            if (s.match(" ")) {
                var parts = text.split(" ");
                text = parts[0];
                for (var i = 1; i < parts.length; ++ i) {
                    text += "\n" + parts[1];
                }
            } else {
                var match = s.match(/[0-9]/);
                if (match) {
                    text = s.slice(0, match.index) + "\n" + s.slice(match.index)
                }
            }
        }

        // week
        text += (isHorizontal ? " " : "\n") + calendar.week;

        // year
        text += ("\n" + calendar.year);

        // month
        s = calendar.year + " " + calendar.month;
        text += (isOverFlow(s) ? "\n" : calendar.separator) + calendar.month;

        dateText.text = text;
    }

    function isOverFlow(string) {
        if (isHorizontal) {
            return false;
        }
        dateText.text = string;
        return dateText.contentWidth >= (root.width - 4);
    }

    Grid {
        id: layout
        anchors.centerIn: parent
        rows: isHorizontal? 1 : 2
        columns: isHorizontal ? 2 : 1
        verticalItemAlignment: Grid.AlignVCenter
        horizontalItemAlignment: Grid.AlignHCenter
        spacing: 0

        StyleBackground {
            id: backGround
            useStyleTransparency: false
            paletteRole: Theme.BrightText
            alpha: mouseArea.containsPress? 0.1 : mouseArea.containsMouse? 0.05 : 0
            borderColor: Theme.BrightText
            borderAlpha: mouseArea.containsPress? 0.1 : mouseArea.containsMouse? 0.05 : 0
            border.width: 0
            radius: Theme.normalRadius
            width: isHorizontal ? dateText.contentWidth + 8 : root.width
            height: isHorizontal ? root.height : dateText.contentHeight + 4

            StyleText {
                id: dateText
                anchors.centerIn: parent
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                width: isHorizontal ? contentWidth : root.width - 4
                height: isHorizontal ? root.height - 4 : contentHeight
                fontSizeMode: isHorizontal ? Text.VerticalFit : Text.HorizontalFit
            }

            MouseArea {
                id: mouseArea
                anchors.fill: parent
                hoverEnabled: true
                acceptedButtons: Qt.LeftButton | Qt.RightButton

                Tooltip {
                    anchors.fill: parent
                    mainText: calendar.tooltipText
                    posFollowCursor: false
                    location: {
                        switch(Widget.container.position) {
                            case Types.Bottom:
                                return Dialog.BottomEdge;
                            case Types.Left:
                                return Dialog.LeftEdge;
                            case Types.Top:
                                return Dialog.TopEdge;
                            case Types.Right:
                                return Dialog.RightEdge;
                        }
                    }
                    margin: 6
                }

                Menu {
                    id: calendarMenu
                    transientParent: parent
                    content: [
                        MenuItem {
                            icon: "document-page-setup-symbolic"
                            text: qsTr("Time and Date Setting")
                            onClicked: function () {
                                AppLauncher.launchAppWithArguments("/usr/share/applications/lingmo-control-center.desktop", ["-m", "Date"]);
                            }
                        }
                    ]
                }

                onClicked: {
                    // TODO: open calendar
                    //console.log("onClicked:", dateText.contentWidth, dateText.contentHeight)
                    if (mouse.button === Qt.RightButton) {
                        calendarMenu.open();
                    } else {
                        calendar.activeCalendar();
                    }
                }
            }
        }

        StyleBackground {
            id: iconBackGround
            useStyleTransparency: false
            paletteRole: Theme.BrightText
            alpha: iconMouseArea.containsPress? 0.1 : iconMouseArea.containsMouse? 0.05 : 0
            borderColor: Theme.BrightText
            borderAlpha: iconMouseArea.containsPress? 0.1 : iconMouseArea.containsMouse? 0.05 : 0
            border.width: 0
            radius: Theme.normalRadius

            // 任务栏内容区域默认高度是44
            property real iconZoom: isHorizontal ? (root.height / 44) * 8 : (root.width / 44) * 8
            width: isHorizontal ? Math.ceil(iconZoom * 4) : Math.ceil(iconZoom * 5)
            height: isHorizontal ? Math.ceil(iconZoom * 5) : Math.ceil(iconZoom * 4)

            Icon {
                id: icon
                anchors.centerIn: parent
                property bool isClearRedpoint: calendar.unreadMsgCount <= 0
                width: Math.ceil(parent.iconZoom * 2)
                height: width
                source: isClearRedpoint ? "lingmo-tool-symbolic" : "lingmo-tool-box-null-symbolic"
                mode: Icon.AutoHighlight
            }

            MouseArea {
                id: iconMouseArea
                anchors.fill: parent
                hoverEnabled: true
                acceptedButtons: Qt.LeftButton | Qt.RightButton

                Tooltip {
                    anchors.fill: parent
                    mainText: String(calendar.notifyIconTooltipText)
                    posFollowCursor: false
                    location: {
                        switch(Widget.container.position) {
                            case Types.Bottom:
                                return Dialog.BottomEdge;
                            case Types.Left:
                                return Dialog.LeftEdge;
                            case Types.Top:
                                return Dialog.TopEdge;
                            case Types.Right:
                                return Dialog.RightEdge;
                        }
                    }
                    margin: 6
                }

                Menu {
                    id: notiMenu
                    transientParent: parent
                    content: [
                        MenuItem {
                            text: qsTr("Open")
                            onClicked: calendar.openSidebar()
                        },
                        MenuItem {
                            icon: "document-page-setup-symbolic"
                            text: qsTr("Set up notification center")
                            onClicked: function () {
                                AppLauncher.launchAppWithArguments("/usr/share/applications/lingmo-control-center.desktop", ["-m", "Notice"]);
                            }
                        }
                    ]
                }

                onClicked: {
                    if (mouse.button === Qt.LeftButton) {
                        calendar.openSidebar();
                    } else if (mouse.button === Qt.RightButton) {
                        notiMenu.open();
                    }
                }
            }
        }
    }
}
