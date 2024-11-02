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
import org.lingmo.menu.starter 1.0
import org.lingmo.quick.items 1.0
import org.lingmo.quick.platform 1.0

WidgetItem {
    id: root
    property bool isHorizontal: Widget.orientation == Types.Horizontal

    Layout.fillWidth: !isHorizontal
    Layout.fillHeight: isHorizontal
    Layout.preferredWidth: isHorizontal ? height : width
    Layout.preferredHeight: isHorizontal ? height : width

    //  更新tooltip
    Widget.tooltip: Widget.tooltip

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true

        StyleBackground {
            anchors.fill: parent
            radius: Theme.normalRadius
            useStyleTransparency: false
            paletteRole: Theme.WindowText
            alpha: parent.containsPress ? 0.16 : parent.containsMouse ? 0.08 : 0
            Tooltip {
                id: tooltip
                anchors.fill: parent
                mainText: Widget.tooltip
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
        }

        Icon {
            anchors.centerIn: parent
            width: parent.height - 8
            height: parent.height - 8

            source: "lingmo-startmenu"
            scale: parent.containsPress ? 0.95 : 1.0
        }

        onClicked: {
            startMenuButton.openStartMenu();
        }
    }

    StartMenuButton {
        id: startMenuButton
    }

    Widget.actions: [
        Action {
            iconName: "battery-full-symbolic"
            text: qsTr("Power Manager")
            onTriggered: function () {
                AppLauncher.runCommand("lingmo-control-center -m power");
            }
        },
        Action {
            iconName: "preferences-system-details-symbolic"
            text: qsTr("About This Computer")
            onTriggered: function () {
                AppLauncher.runCommand("lingmo-control-center -m about");
            }
        },
        Action {
            iconName: "network-wired-acquiring-symbolic"
            text: qsTr("Network Settings")
            onTriggered: function () {
                AppLauncher.runCommand("lingmo-control-center -m netconnect");
            }
        },
        Action {
            isSeparator: true
        },
        Action {
            iconName: "system-monitor-app-symbolic"
            text: qsTr("System Monitor")
            onTriggered: function () {
                AppLauncher.launchApp("/usr/share/applications/lingmo-system-monitor.desktop");
            }
        },
        Action {
            iconName: "system-settings-symbolic"
            text: qsTr("Control Center")
            onTriggered: function () {
                AppLauncher.launchApp("/usr/share/applications/lingmo-control-center.desktop");
            }
        },
        Action {
            iconName: "folder-symbolic"
            text: qsTr("File Manager")
            onTriggered: function () {
                // TODO: 使用标准协议打开文件管理器，保持不同桌面环境的兼容性
                AppLauncher.launchApp("/usr/share/applications/explor.desktop");
            }
        },
        Action {
            iconName: "search-symbolic"
            text: qsTr("Search")
            onTriggered: function () {
                AppLauncher.launchApp("/usr/share/applications/lingmo-search-menu.desktop");
            }
        },
        Action {
            iconName: "terminal-app-symbolic"
            text: qsTr("Open Terminal")
            onTriggered: function () {
                AppLauncher.runCommand("/usr/bin/x-terminal-emulator");
            }
        },
        Action {
            isSeparator: true
        },
        Action {
            iconName: "user-identity"
            text: qsTr("Switch User or Log Out")
            subActions: startMenuButton.userActions
            enabled: startMenuButton.userActions.length > 0
        },
        Action {
            iconName: "exit-symbolic"
            text: qsTr("Power Options")
            subActions: startMenuButton.powerActions
            enabled: startMenuButton.powerActions.length > 0
        },
        Action {
            iconName: "computer-symbolic"
            text: qsTr("Show Desktop")
            onTriggered: function () {
                startMenuButton.showDesktop();
            }
        },
        Action {
            isSeparator: true
        },
        Action {
            iconName: "view-grid-symbolic"
            text: qsTr("All Applications")
            onTriggered: function () {
                startMenuButton.openStartMenu();
            }
        }
    ]
}
