/*
    SPDX-FileCopyrightText: 2019-2020 Aditya Mehra <aix.m@outlook.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

import QtQuick 2.14
import QtQuick.Layouts 1.14
import QtQuick.Controls 2.14 as Controls
import QtQuick.Window 2.14
import org.kde.lingmo.core 2.0 as LingmoCore
import org.kde.kquickcontrolsaddons 2.0
import org.kde.lingmoui 2.12 as LingmoUI
import org.kde.lingmo.private.volume 0.1
import org.kde.mycroft.bigscreen 1.0 as BigScreen

import "delegates" as Delegates
import "views" as Views

FocusScope {
    id: mainFlick

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
            text: "Audio Settings"
        }
    }

    Item {
        id: footerMain
        anchors.left: parent.left
        anchors.right: settingsView.left
        anchors.leftMargin: -LingmoUI.Units.largeSpacing
        anchors.bottom: parent.bottom
        implicitHeight: LingmoUI.Units.gridUnit * 2

        Controls.Button {
            id: kcmcloseButton
            implicitHeight: LingmoUI.Units.gridUnit * 2
            anchors.left: parent.left
            anchors.right: parent.right

            background: Rectangle {
                color: kcmcloseButton.activeFocus ? LingmoUI.Theme.highlightColor : LingmoUI.Theme.backgroundColor
            }

            contentItem: Item {
                RowLayout {
                    anchors.centerIn: parent
                    LingmoUI.Icon {
                        Layout.preferredWidth: LingmoCore.Units.iconSizes.small
                        Layout.preferredHeight: LingmoCore.Units.iconSizes.small
                        source: "window-close"
                    }
                    Controls.Label {
                        text: i18n("Exit")
                    }
                }
            }

            Keys.onUpPressed: root.activateDeviceView()

            onClicked: {
                Window.window.close()
            }

            Keys.onReturnPressed: {
                Window.window.close()
            }
        }
    }


    SourceModel {
        id: paSourceModel
    }

    SinkModel {
        id: paSinkModel
    }

    Item {
        anchors.top: headerAreaTop.bottom
        anchors.topMargin: LingmoUI.Units.largeSpacing * 2
        width: parent.width - settingsView.width
        anchors.bottom: footerMain.top
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

            BigScreen.TileView {
                id: sinkView
                model: paSinkModel
                focus: true
                Layout.alignment: Qt.AlignTop
                title: i18n("Playback Devices")
                currentIndex: 0
                onActiveFocusChanged: {
                    if(activeFocus){
                        contentLayout.currentSection = sinkView
                        settingsViewDetails.model = sinkView.model
                        settingsViewDetails.positionViewAtIndex(currentIndex, ListView.Center);
                        settingsViewDetails.checkPlayBack = true
                        settingsViewDetails.typeDevice = "sink"
                    }
                }
                delegate: Delegates.AudioDelegate {
                    isPlayback: true
                    type: "sink"
                }
                navigationDown: sourceView.visible ? sourceView : kcmcloseButton

                onCurrentItemChanged: {
                    settingsViewDetails.currentIndex = sinkView.currentIndex
                    settingsViewDetails.positionViewAtIndex(sinkView.currentIndex, ListView.Center);
                }
            }

            Item {
                id: extraSpacing
                Layout.preferredHeight: LingmoUI.Units.largeSpacing
                Layout.fillWidth: true
            }

            BigScreen.TileView {
                id: sourceView
                model: paSourceModel
                title: i18n("Recording Devices")
                currentIndex: 0
                focus: false
                Layout.alignment: Qt.AlignTop
                visible: sourceView.view.count > 0 ? 1 : 0
                onActiveFocusChanged: {
                    if(activeFocus){
                        contentLayout.currentSection = sourceView
                        settingsViewDetails.model = sourceView.model
                        settingsViewDetails.positionViewAtIndex(currentIndex, ListView.Center);
                        settingsViewDetails.checkPlayBack = false
                        settingsViewDetails.typeDevice = "source"
                    }
                }
                delegate: Delegates.AudioDelegate {
                    isPlayback: false
                    type: "source"
                }
                navigationUp: sinkView
                navigationDown: kcmcloseButton

                onCurrentItemChanged: {
                    settingsViewDetails.currentIndex = sourceView.currentIndex
                    settingsViewDetails.positionViewAtIndex(currentIndex, ListView.Center);
                }
            }

            Component.onCompleted: {
                sinkView.forceActiveFocus();
            }

            Connections {
                target: root
                onActivateDeviceView: {
                    sinkView.forceActiveFocus();
                }
            }

            Item {
                Layout.preferredHeight: LingmoUI.Units.gridUnit * 5
            }
        }
    }
    
    LingmoUI.Separator {
        id: viewSept
        anchors.right: settingsView.left
        anchors.top: settingsView.top
        anchors.bottom: settingsView.bottom
        width: 1
    }
    
    Rectangle {
        id: settingsView
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.rightMargin: -LingmoUI.Units.smallSpacing
        height: parent.height
        width: LingmoUI.Units.gridUnit * 15
        color: LingmoUI.Theme.backgroundColor

        ListView {
            id: settingsViewDetails
            anchors.fill: parent
            anchors.topMargin: parent.height * 0.075
            layoutDirection: Qt.LeftToRight
            orientation: ListView.Horizontal
            snapMode: ListView.SnapOneItem;
            highlightRangeMode: ListView.StrictlyEnforceRange
            highlightFollowsCurrentItem: true
            spacing: LingmoUI.Units.largeSpacing
            clip: true
            interactive: false
            implicitHeight: settingsView.implicitHeight
            currentIndex: 0
            property bool checkPlayBack
            property string typeDevice
            delegate: SettingsItem {
                isPlayback: settingsViewDetails.checkPlayBack
                type: settingsViewDetails.typeDevice
            }
        }
    }
}
