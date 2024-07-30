/*
    SPDX-FileCopyrightText: 2019 Aditya Mehra <aix.m@outlook.com>
    SPDX-FileCopyrightText: 2019 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.14
import QtQuick.Layouts 1.14
import QtQuick.Controls 2.14
import org.kde.lingmo.components 3.0 as LingmoComponents
import org.kde.lingmo.extras 2.0 as LingmoExtras
import org.kde.lingmoui 2.12 as LingmoUI
import org.kde.mycroft.bigscreen 1.0 as BigScreen
import Qt5Compat.GraphicalEffects
import org.kde.lingmo.private.volume 0.1

import "delegates" as Delegates
import "code/icon.js" as Icon

Item {
    id: delegateSettingsItem
    property bool isPlayback: type.substring(0, 4) == "sink"
    property string type
    readonly property var currentPort: Ports[ActivePortIndex]
    readonly property ListView listView: ListView.view
    width: listView.width
    height: listView.height

    onActiveFocusChanged: {
        if(activeFocus){
            if(model.PulseObject.default){
                volObj.forceActiveFocus()
            } else {
                setDefBtn.forceActiveFocus()
            }
        }
    }
    
    Keys.onBackPressed: {
        backBtnSettingsItem.clicked()
    }

    Item {
        id: colLayoutSettingsItem
        clip: true
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            bottom: footerAreaSettingsSept.top
            margins: LingmoUI.Units.largeSpacing * 2
        }
        
        Item {
            anchors.centerIn: parent
            width: parent.width
            height: dIcon.height + label1.paintedHeight + label2.paintedHeight + lblSept.height + lblSept2.height + setDefBtn.height + (volObj.height * 2)

            LingmoUI.Icon {
                id: dIcon
                anchors.top: parent.top
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width
                height: width / 4
                source: Icon.name(Volume, Muted, isPlayback ? "audio-volume" : "microphone-sensitivity")
            }

            LingmoUI.Heading {
                id: label1
                width: parent.width
                anchors.top: dIcon.bottom
                anchors.topMargin: LingmoUI.Units.largeSpacing
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
                level: 2
                maximumLineCount: 2
                elide: Text.ElideRight
                color: LingmoUI.Theme.textColor
                font.pixelSize: textMetrics.font.pixelSize * 1.25
                text: currentPort.description
            }
            
            LingmoUI.Heading {
                id: label2
                width: parent.width
                anchors.top: label1.bottom
                anchors.topMargin: LingmoUI.Units.largeSpacing
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
                level: 2
                maximumLineCount: 2
                elide: Text.ElideRight
                color: LingmoUI.Theme.textColor
                font.pixelSize: textMetrics.font.pixelSize * 0.9
                text: Description
            }
            
            LingmoUI.Separator {
                id: lblSept
                anchors.top: label2.bottom
                anchors.topMargin: LingmoUI.Units.largeSpacing
                height: 1
                width: parent.width
            }
            
            Button {
                id: setDefBtn
                //enabled: PulseObject.default ? 1 : 0
                KeyNavigation.up: backBtnSettingsItem
                KeyNavigation.down: volObj
                width: parent.width
                height: LingmoUI.Units.gridUnit * 2
                anchors.top: lblSept.bottom
                anchors.topMargin: LingmoUI.Units.smallSpacing
                
                Keys.onReturnPressed: {
                    model.PulseObject.default = true;
                    listView.currentIndex = index
                    volObj.forceActiveFocus()
                }
                
                background: Rectangle {
                    color: setDefBtn.activeFocus ? LingmoUI.Theme.highlightColor : LingmoUI.Theme.backgroundColor
                }
                
                contentItem: Item {
                    RowLayout {
                        anchors.centerIn: parent
                        LingmoUI.Icon {
                            Layout.preferredWidth: LingmoUI.Units.iconSizes.medium
                            Layout.preferredHeight: LingmoUI.Units.iconSizes.medium
                            source: Qt.resolvedUrl("images/green-tick-thick.svg")
                            enabled: model.PulseObject.default  ? 1 : 0
                        }
                    }
                }
                
                onClicked:  {
                    model.PulseObject.default = true;
                    listView.currentIndex = index
                }
            }
            
            LingmoUI.Separator {
                id: lblSept2
                anchors.top: setDefBtn.bottom
                anchors.topMargin: LingmoUI.Units.smallSpacing
                height: 1
                width: parent.width
            }
            
            Item {
                anchors.top: lblSept2.bottom
                anchors.topMargin: LingmoUI.Units.largeSpacing
                width: parent.width
                height: volObj.height

                Delegates.VolumeObject {
                    id: volObj
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter

                    Keys.onRightPressed: {
                        increaseVal()
                    }
                    Keys.onLeftPressed: {
                        decreaseVal()
                    }

                    Keys.onDownPressed: {
                        backBtnSettingsItem.forceActiveFocus()
                    }
                }

                LingmoExtras.Highlight {
                    z: -2
                    anchors.fill: parent
                    anchors.margins: -LingmoUI.Units.gridUnit / 4
                    visible: volObj.activeFocus ? 1 : 0
                }
            }
        }
    }

    LingmoUI.Separator {
        id: footerAreaSettingsSept
        anchors.bottom: footerAreaSettingsItem.top
        anchors.bottomMargin: LingmoUI.Units.largeSpacing
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.leftMargin: LingmoUI.Units.largeSpacing * 2
        anchors.rightMargin: LingmoUI.Units.largeSpacing * 2
        height: 1
    }

    RowLayout {
        id: footerAreaSettingsItem
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: LingmoUI.Units.largeSpacing * 2
        height: LingmoUI.Units.gridUnit * 2

        LingmoComponents.Button {
            id: backBtnSettingsItem
            icon.name: "arrow-left"
            Layout.alignment: Qt.AlignLeft
            KeyNavigation.up: volObj

            LingmoExtras.Highlight {
                z: -2
                anchors.fill: parent
                anchors.margins: -LingmoUI.Units.gridUnit / 4
                visible: backBtnSettingsItem.activeFocus ? 1 : 0
            }

            Keys.onReturnPressed: {
                clicked()
            }

            onClicked: {
                if(type == "sink"){
                    sinkView.forceActiveFocus()
                } else if (type == "source") {
                    sourceView.forceActiveFocus()
                }
            }
        }

        Label {
            id: backbtnlabelHeading
            text: i18n("Press the [←] Back button to return to device selection")
            wrapMode: Text.WordWrap
            maximumLineCount: 2
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignRight
        }
    }
}

