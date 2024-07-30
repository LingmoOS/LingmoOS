// SPDX-FileCopyrightText: 2023 Devin Lin <devin@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import QtQuick.Window
import QtQuick.Layouts
import QtQuick.Controls as QQC2

import org.kde.lingmoui 2.20 as LingmoUI

import org.kde.lingmo.components 3.0 as PC3
import org.kde.lingmo.private.mobileshell as MobileShell

Item {
    id: root

    property real leftMargin
    property real rightMargin
    property real bottomMargin
    property var homeScreen

    MouseArea {
        id: closeSettings

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: settingsBar.top

        onClicked: {
            root.homeScreen.settingsOpen = false;
        }
    }

    Item {
        id: settingsBar
        height: settingsOptions.implicitHeight

        LingmoUI.Theme.inherit: false
        LingmoUI.Theme.colorSet: LingmoUI.Theme.Complementary

        anchors.left: parent.left
        anchors.leftMargin: root.leftMargin
        anchors.right: parent.right
        anchors.rightMargin: parent.rightMargin
        anchors.bottom: parent.bottom
        anchors.bottomMargin: LingmoUI.Units.largeSpacing + root.bottomMargin

        RowLayout {
            id: settingsOptions
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: LingmoUI.Units.largeSpacing

            PC3.ToolButton {
                opacity: 0.9
                implicitHeight: LingmoUI.Units.gridUnit * 4
                implicitWidth: LingmoUI.Units.gridUnit * 5

                contentItem: ColumnLayout {
                    spacing: LingmoUI.Units.largeSpacing

                    LingmoUI.Icon {
                        Layout.alignment: Qt.AlignHCenter | Qt.AlignBottom
                        implicitWidth: LingmoUI.Units.iconSizes.smallMedium
                        implicitHeight: LingmoUI.Units.iconSizes.smallMedium
                        source: 'edit-image'
                    }

                    QQC2.Label {
                        Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
                        text: i18n('Wallpapers')
                        font.bold: true
                    }
                }

                onClicked: {
                    root.homeScreen.settingsOpen = false;
                    wallpaperSelectorLoader.active = true;
                }
            }

            PC3.ToolButton {
                opacity: 0.9
                implicitHeight: LingmoUI.Units.gridUnit * 4
                implicitWidth: LingmoUI.Units.gridUnit * 5

                contentItem: ColumnLayout {
                    spacing: LingmoUI.Units.largeSpacing

                    LingmoUI.Icon {
                        Layout.alignment: Qt.AlignHCenter | Qt.AlignBottom
                        implicitWidth: LingmoUI.Units.iconSizes.smallMedium
                        implicitHeight: LingmoUI.Units.iconSizes.smallMedium
                        source: 'settings-configure'
                    }

                    QQC2.Label {
                        Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
                        text: i18n('Settings')
                        font.bold: true
                    }
                }

                onClicked: {
                    root.homeScreen.settingsOpen = false;
                    root.homeScreen.openContainmentSettings();
                }
            }
        }
    }

    Loader {
        id: wallpaperSelectorLoader
        asynchronous: true
        active: false

        onLoaded: {
            wallpaperSelectorLoader.item.open();
        }

        sourceComponent: MobileShell.WallpaperSelector {
            horizontal: root.width > root.height
            edge: horizontal ? Qt.LeftEdge : Qt.BottomEdge
            bottomMargin: root.bottomMargin
            leftMargin: root.leftMargin
            rightMargin: root.rightMargin
            onClosed: {
                wallpaperSelectorLoader.active = false;
            }
            onWallpaperSettingsRequested: {
                close();
                root.homeScreen.openContainmentSettings();
            }
        }
    }
}
