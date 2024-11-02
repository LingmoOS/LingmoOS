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

import QtQml 2.12
import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.5

import org.lingmo.menu.core 1.0
import org.lingmo.menu.utils 1.0
import org.lingmo.menu.extension 1.0
import org.lingmo.quick.platform 1.0 as Platform
import org.lingmo.quick.items 1.0 as LingmoUIItems

Item {
    ColumnLayout {
        anchors.fill: parent
        anchors.topMargin: 12
        anchors.bottomMargin: 12
        spacing: 4

        LingmoUIItems.Button {
            id: fullScreenButton
            visible: !isLiteMode
            background.paletteRole: Platform.Theme.WindowText
            background.alpha: containsPress ? 0.15 : containsMouse ? 0.08 : 0.0

            Layout.preferredWidth: 36
            Layout.preferredHeight: 36
            Layout.alignment: Qt.AlignHCenter

            LingmoUIItems.Tooltip {
                anchors.fill: parent
                mainText: mainWindow.isFullScreen ? qsTr("Contract") : qsTr("Expand")
                posFollowCursor: true
                margin: 6
            }

            onClicked: {
                if (mainWindow.isFullScreen) {
                    root.currentSearchText = searchInputBar.text;
                    mainWindow.exitFullScreen();
                } else {
                    root.currentSearchText = appPage.search.searchInputBar.text;
                    mainWindow.isFullScreen = true;
                }
            }
            Keys.onPressed: {
                if (event.key === Qt.Key_Enter || event.key === Qt.Key_Return) {
                    if (mainWindow.isFullScreen) {
                        root.currentSearchText = searchInputBar.text;
                        mainWindow.exitFullScreen();
                    } else {
                        root.currentSearchText = appPage.search.searchInputBar.text;
                        mainWindow.isFullScreen = true;
                    }
                }
            }

            background.radius: Platform.Theme.minRadius
            icon.mode: LingmoUIItems.Icon.AutoHighlight
            icon.source: mainWindow.isFullScreen ? "view-restore-symbolic" : "view-fullscreen-symbolic"
        }

        Item {
            id: blankSpace
            Layout.fillHeight: true
            Layout.fillWidth: true
            SidebarButtonUtils {
                id: totalUtils
            }
        }

        LingmoUIItems.Button {
            id: userInfoButton
            background.paletteRole: Platform.Theme.WindowText
            background.alpha: containsPress ? 0.15 : containsMouse ? 0.08 : 0.0

            Layout.preferredWidth: 36
            Layout.preferredHeight: 36
            Layout.alignment: Qt.AlignHCenter
            icon.width: 24
            icon.height: 24

            LingmoUIItems.Tooltip {
                anchors.fill: parent
                mainText: totalUtils.realName
                posFollowCursor: true
                margin: 6
            }

            onClicked: totalUtils.openUserCenter()
            Keys.onPressed: {
                if (event.key === Qt.Key_Enter || event.key === Qt.Key_Return) {
                    totalUtils.openUserCenter();
                }
            }

            background.radius: Platform.Theme.minRadius
            icon.mode: LingmoUIItems.Icon.AutoHighlight
            icon.source: totalUtils.iconFile
        }

        LingmoUIItems.Button {
            id: computerButton
            background.paletteRole: Platform.Theme.WindowText
            background.alpha: containsPress ? 0.15 : containsMouse ? 0.08 : 0.0

            Layout.preferredWidth: 36
            Layout.preferredHeight: 36
            Layout.alignment: Qt.AlignHCenter

            LingmoUIItems.Tooltip {
                anchors.fill: parent
                mainText: qsTr("Computer")
                posFollowCursor: true
                margin: 6
            }

            onClicked: {
                totalUtils.openPeonyComputer();
            }
            Keys.onPressed: {
                if (event.key === Qt.Key_Enter || event.key === Qt.Key_Return) {
                    totalUtils.openPeonyComputer();
                }
            }

            background.radius: Platform.Theme.minRadius
            icon.mode: LingmoUIItems.Icon.AutoHighlight
            icon.source: "computer-symbolic"
        }

        LingmoUIItems.Button {
            id: setButton
            background.paletteRole: Platform.Theme.WindowText
            background.alpha: containsPress ? 0.15 : containsMouse ? 0.08 : 0.0

            Layout.preferredWidth: 36
            Layout.preferredHeight: 36
            Layout.alignment: Qt.AlignHCenter

            LingmoUIItems.Tooltip {
                anchors.fill: parent
                mainText: qsTr("Control center")
                posFollowCursor: true
                margin: 6
            }

            onClicked: {
                totalUtils.openControlCenter();
            }
            Keys.onPressed: {
                if (event.key === Qt.Key_Enter || event.key === Qt.Key_Return) {
                    totalUtils.openControlCenter();
                }
            }

            background.radius: Platform.Theme.minRadius
            icon.mode: LingmoUIItems.Icon.AutoHighlight
            icon.source: "applications-system-symbolic"
        }


        LingmoUIItems.Button {
            id: powerButton
            background.paletteRole: Platform.Theme.WindowText
            background.alpha: containsPress ? 0.15 : containsMouse ? 0.08 : 0.0

            Layout.preferredWidth: 36
            Layout.preferredHeight: 36
            Layout.alignment: Qt.AlignHCenter

            LingmoUIItems.Tooltip {
                anchors.fill: parent
                mainText: powerButtonBase.toolTip
                posFollowCursor: true
                margin: 6
            }

            PowerButton {
                id: powerButtonBase
            }

            acceptedButtons: Qt.LeftButton | Qt.RightButton
            property int spacingFromMenu: 16

            Keys.onPressed: {
                if (event.key === Qt.Key_Enter || event.key === Qt.Key_Return) {
                    powerButtonBase.clicked(true, 0, 0, mainWindow.isFullScreen);
                }
            }

            onClicked: {
                var buttonPosition = mapToGlobal(width, height);
                powerButtonBase.clicked(mouse.button === Qt.LeftButton, buttonPosition.x + spacingFromMenu, buttonPosition.y + spacingFromMenu, mainWindow.isFullScreen);
            }

            background.radius: Platform.Theme.minRadius
            icon.mode: LingmoUIItems.Icon.AutoHighlight
            icon.source: powerButtonBase.icon
        }
    }
}
