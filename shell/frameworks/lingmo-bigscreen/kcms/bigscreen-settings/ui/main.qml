/*
    SPDX-FileCopyrightText: 2020 Aditya Mehra <aix.m@outlook.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

*/

import QtQuick.Layouts 1.14
import QtQuick 2.14
import QtQuick.Window 2.14
import QtQuick.Controls 2.14
import org.kde.lingmo.core 2.0 as LingmoCore
import org.kde.lingmoui as LingmoUI
import org.kde.kcmutils as KCM
import org.kde.mycroft.bigscreen 1.0 as BigScreen
import "delegates" as Delegates

KCM.SimpleKCM {
    id: root

    title: i18n("Appearance")
    background: null
    leftPadding: LingmoUI.Units.smallSpacing
    topPadding: 0
    rightPadding: LingmoUI.Units.smallSpacing
    bottomPadding: 0

    Component.onCompleted: {
        desktopThemeView.forceActiveFocus();
    }

    function setTheme(packageName){
        kcm.themeName = packageName
    }

    function setOption(type, result){
        if(type == "coloredTile"){
            kcm.setUseColoredTiles(result);
        }
        if(type == "exapandableTile"){
            kcm.setUseExpandingTiles(result);
        }
        if(type == "mycroftIntegration"){
            kcm.setMycroftIntegrationActive(result);
        }
        if(type == "pmInhibition"){
            kcm.setPmInhibitionActive(result);
        }
    }

    contentItem: FocusScope {

        Rectangle {
            id: headerAreaTop
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.leftMargin: -LingmoUI.Units.largeSpacing
            anchors.rightMargin: -LingmoUI.Units.largeSpacing
            height: parent.height * 0.075
            z: 10
            gradient: Gradient {
                GradientStop { position: 0.1; color: Qt.rgba(0, 0, 0, 0.5) }
                GradientStop { position: 0.9; color: Qt.rgba(0, 0, 0, 0.25) }
            }

            LingmoUI.Heading {
                level: 1
                anchors.fill: parent
                anchors.topMargin: LingmoUI.Units.largeSpacing
                anchors.leftMargin: LingmoUI.Units.largeSpacing * 2
                anchors.bottomMargin: LingmoUI.Units.largeSpacing
                color: LingmoUI.Theme.textColor
                text: "Bigscreen Settings"
            }
        }

        Item {
            id: footerMain
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.leftMargin: -LingmoUI.Units.largeSpacing
            anchors.bottom: parent.bottom
            implicitHeight: LingmoUI.Units.gridUnit * 2

            Button {
                id: kcmcloseButton
                implicitHeight: LingmoUI.Units.gridUnit * 2
                width: deviceTimeSettingsArea.opened ? (root.width + LingmoUI.Units.largeSpacing) - deviceTimeSettingsArea.width : root.width + LingmoUI.Units.largeSpacing
                KeyNavigation.up: desktopThemeView
                KeyNavigation.down: mycroftIntegrationDelegate

                background: Rectangle {
                    color: kcmcloseButton.activeFocus ? LingmoUI.Theme.highlightColor : LingmoUI.Theme.backgroundColor
                }

                contentItem: Item {
                    RowLayout {
                        anchors.centerIn: parent
                        LingmoUI.Icon {
                            Layout.preferredWidth: LingmoUI.Units.iconSizes.small
                            Layout.preferredHeight: LingmoUI.Units.iconSizes.small
                            source: "window-close"
                        }
                        Label {
                            text: i18n("Exit")
                        }
                    }
                }

                onClicked: {
                    Window.window.close()
                }

                Keys.onReturnPressed: {
                    Window.window.close()
                }
            }
        }

        Item {
            anchors.left: parent.left
            anchors.leftMargin: LingmoUI.Units.largeSpacing
            anchors.top: headerAreaTop.bottom
            anchors.topMargin: LingmoUI.Units.largeSpacing * 2
            anchors.bottom: footerMain.top
            width: deviceTimeSettingsArea.opened ? parent.width - deviceTimeSettingsArea.width : parent.width
            clip: true

            ColumnLayout {
                id: contentLayout
                width: parent.width
                property Item currentSection
                y: currentSection ? (currentSection.y > parent.height / 2 ? -currentSection.y + LingmoUI.Units.gridUnit * 3 : 0) : 0
                anchors.left: parent.left
                anchors.leftMargin: LingmoUI.Units.largeSpacing

                Behavior on y {
                    NumberAnimation {
                        duration: LingmoUI.Units.longDuration * 2
                        easing.type: Easing.InOutQuad
                    }
                }

                LingmoUI.Heading {
                    id: launcherLookHeader
                    text: i18n("Launcher Appearance")
                    layer.enabled: true
                    color: LingmoUI.Theme.textColor
                }

                Flickable {
                    id: topContentAreaLayout
                    Layout.fillWidth: true
                    Layout.preferredHeight: topContentArea.implicitHeight
                    contentWidth: topContentArea.implicitWidth
                    contentHeight: height

                    Behavior on contentX {
                        NumberAnimation {
                            duration: LingmoUI.Units.longDuration * 2
                            easing.type: Easing.InOutQuad
                        }
                    }

                    RowLayout {
                        id: topContentArea
                        height: parent.height

                        Delegates.LocalSettingDelegate {
                            id: mycroftIntegrationDelegate
                            implicitWidth: desktopThemeView.view.cellWidth * 3
                            implicitHeight: desktopThemeView.view.cellHeight
                            isChecked: kcm.mycroftIntegrationActive() ? 1 : 0
                            name: "Mycroft Integration"
                            customType: "mycroftIntegration"
                            KeyNavigation.up: kcmcloseButton
                            KeyNavigation.right: pmInhibitionDelegate
                            KeyNavigation.down: desktopThemeView
                            onActiveFocusChanged: {
                                if(activeFocus){
                                    contentLayout.currentSection = topContentArea
                                    topContentAreaLayout.contentX = mycroftIntegrationDelegate.x
                                }
                            }
                        }

                        Delegates.LocalSettingDelegate {
                            id: pmInhibitionDelegate
                            implicitWidth: desktopThemeView.view.cellWidth * 3
                            implicitHeight: desktopThemeView.view.cellHeight
                            isChecked: kcm.pmInhibitionActive() ? 1 : 0
                            name: i18n("Power Inhibition")
                            customType: "pmInhibition"
                            KeyNavigation.up: kcmcloseButton
                            KeyNavigation.right: coloredTileDelegate
                            KeyNavigation.left: mycroftIntegrationDelegate
                            KeyNavigation.down: desktopThemeView
                            onActiveFocusChanged: {
                                if(activeFocus){
                                    contentLayout.currentSection = topContentArea
                                    topContentAreaLayout.contentX = pmInhibitionDelegate.x
                                }
                            }
                        }

                        Delegates.LocalSettingDelegate {
                            id: coloredTileDelegate
                            implicitWidth: desktopThemeView.view.cellWidth * 3
                            implicitHeight: desktopThemeView.view.cellHeight
                            isChecked: kcm.useColoredTiles() ? 1 : 0
                            name: i18n("Colored Tiles")
                            customType: "coloredTile"
                            KeyNavigation.up: kcmcloseButton
                            KeyNavigation.left: pmInhibitionDelegate
                            KeyNavigation.right: expandableTileDelegate
                            KeyNavigation.down: desktopThemeView
                            onActiveFocusChanged: {
                                if(activeFocus){
                                    contentLayout.currentSection = topContentArea
                                    topContentAreaLayout.contentX = coloredTileDelegate.x
                                }
                            }
                        }

                        Delegates.LocalSettingDelegate {
                            id: expandableTileDelegate
                            implicitWidth: desktopThemeView.cellWidth * 3
                            implicitHeight: desktopThemeView.cellHeight
                            isChecked: kcm.useExpandingTiles() ? 1 : 0
                            name: i18n("Expanding Tiles")
                            customType: "exapandableTile"
                            KeyNavigation.up: kcmcloseButton
                            KeyNavigation.left: coloredTileDelegate
                            KeyNavigation.right: timeDateSettingsDelegate
                            KeyNavigation.down: desktopThemeView
                            onActiveFocusChanged: {
                                if(activeFocus){
                                    contentLayout.currentSection = topContentArea
                                    topContentAreaLayout.contentX = expandableTileDelegate.x
                                }
                            }
                        }

                        Delegates.TimeDelegate {
                            id: timeDateSettingsDelegate
                            implicitWidth: desktopThemeView.cellWidth * 3
                            implicitHeight: desktopThemeView.cellHeight
                            name: i18n("Adjust Date & Time")
                            KeyNavigation.up: kcmcloseButton
                            KeyNavigation.left: expandableTileDelegate
                            KeyNavigation.down: desktopThemeView
                            onActiveFocusChanged: {
                                if(activeFocus){
                                    contentLayout.currentSection = topContentArea
                                    topContentAreaLayout.contentX = timeDateSettingsDelegate.x
                                }
                            }
                        }
                    }
                }

                Item {
                    Layout.fillWidth: true
                    Layout.preferredHeight: LingmoUI.Units.largeSpacing * 2
                }

                BigScreen.TileView {
                    id: desktopThemeView
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.alignment: Qt.AlignTop
                    focus: true
                    model: kcm.themeListModel
                    view.cacheBuffer: parent.width * 2
                    title: i18n("General Appearance")
                    navigationUp: mycroftIntegrationDelegate
                    navigationDown: kcmcloseButton
                    enabled: !deviceTimeSettingsArea.opened
                    delegate: Delegates.ThemeDelegate {
                        text: model.display
                    }

                    onActiveFocusChanged: {
                        if(activeFocus){
                            contentLayout.currentSection = desktopThemeView
                        }
                    }

                    Behavior on x {
                        NumberAnimation {
                            duration: LingmoUI.Units.longDuration * 2
                            easing.type: Easing.InOutQuad
                        }
                    }
                }
            }
        }

        DeviceTimeSettings {
            id: deviceTimeSettingsArea
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.right: parent.right
            anchors.rightMargin: -LingmoUI.Units.smallSpacing

            property bool opened: false

            width: parent.width / 3.5
            visible: opened
            enabled: opened
        }
    }
}

