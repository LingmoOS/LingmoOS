// SPDX-FileCopyrightText: 2021 Anupam Basak <anupam.basak27@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2021 Aditya Mehra <aix.m@outlook.com>
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import org.kde.lingmo.components 3.0 as LingmoComponents
import org.kde.lingmoui 2.9 as LingmoUI

import PicoWizard 1.0

ModuleMediaCenter {
    id: root
    property var activeFocusedElement

    function switchTextByFocus(){
        switch(root.activeFocusedElement) {
            case "searchFieldFocusBox":
                return [qsTr('Remote: Press the "Select|OK" button to search a timezone'), qsTr('Keyboard: Press the "Enter" button to search a timezone')]
            case "tzContainer":
                return [qsTr('Remote: Press the "Select|OK" button to start selecting a timezone from the list'), qsTr('Keyboard: Press the "Enter" button to start selecting a timezone from the list')]
            case "timezoneNextBtn":
                return [qsTr('Remote: Press the "Select|OK" button to continue'), qsTr('Keyboard: Press the "Enter" button to continue')]
            case "tzListView":
                return [qsTr('Remote: Use "Up|Down" buttons to navigate, "Select|OK" button to select highlighted timezone'), qsTr('Keyboard: Use "Up|Down" buttons to navigate, "Enter" button to select highlighted timezone')]
            case "timezoneSkipButton":
                return [qsTr('Remote: Press the "Select|OK" button to skip timezone setup and continue'), qsTr('Keyboard: Press the "Enter" button to skip timezone setup and continue')]
        }
    }

    delegate: Item {

        Component.onCompleted: {
            tzContainer.forceActiveFocus()
        }

        ColumnLayout {
            anchors {
                top: parent.top
                bottom: parent.bottom
                bottomMargin: 16
                horizontalCenter: parent.horizontalCenter
            }

            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: LingmoUI.Units.gridUnit * 6
                Layout.alignment: Qt.AlignHCenter

                Rectangle {
                    color: LingmoUI.Theme.activeTextColor
                    radius: 4
                    width: LingmoUI.Units.gridUnit * 8
                    height: LingmoUI.Units.gridUnit * 1
                    anchors.top: parent.top
                    x: infoRectContent.x
                    y: -2
                    z: 2

                    Label {
                        anchors.fill: parent
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        minimumPixelSize: 2
                        font.pixelSize: 72
                        maximumLineCount: 3
                        fontSizeMode: Text.Fit
                        wrapMode: Text.WordWrap
                        text: qsTr("Setting Up Your Timezone")
                        color: LingmoUI.Theme.textColor
                    }
                }

                Rectangle {
                    id: infoRectContent
                    color: LingmoUI.Theme.backgroundColor
                    radius: 4
                    width: parent.width
                    height: LingmoUI.Units.gridUnit * 5
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: LingmoUI.Units.smallSpacing
                    z: 1

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: LingmoUI.Units.largeSpacing

                        RowLayout {
                            Layout.fillWidth: true
                            Layout.fillHeight: true

                            Item {
                                Layout.preferredWidth: LingmoUI.Units.gridUnit * 2
                                Layout.fillHeight: true

                                LingmoUI.Icon {
                                    anchors.fill: parent
                                    source: timezoneModule.dir() + "/assets/remote-ok.svg"
                                }
                            }

                            LingmoUI.Separator {
                                Layout.preferredWidth: 1
                                Layout.fillHeight: true
                            }

                            Label {
                                id: labelButtonInfo
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                                font.weight: Font.Light
                                minimumPixelSize: 2
                                font.pixelSize: 25
                                maximumLineCount: 2
                                fontSizeMode: Text.Fit
                                wrapMode: Text.WordWrap
                                text: switchTextByFocus()[0]
                                color: LingmoUI.Theme.textColor
                            }
                        }

                        LingmoUI.Separator {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 1
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            Layout.fillHeight: true

                            Item {
                                Layout.preferredWidth: LingmoUI.Units.gridUnit * 2
                                Layout.fillHeight: true

                                LingmoUI.Icon {
                                    anchors.fill: parent
                                    source: timezoneModule.dir() + "/assets/keyboard-ok.svg"
                                }
                            }

                            LingmoUI.Separator {
                                Layout.preferredWidth: 1
                                Layout.fillHeight: true
                            }

                            Label {
                                id: labelButtonInfo2
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                                font.weight: Font.Light
                                minimumPixelSize: 2
                                font.pixelSize: 25
                                maximumLineCount: 2
                                fontSizeMode: Text.Fit
                                wrapMode: Text.WordWrap
                                text: switchTextByFocus()[1]
                                color: LingmoUI.Theme.textColor
                            }
                        }
                    }
                }
            }

            Rectangle {
                id: tzContainer
                objectName: "tzContainer"
                Layout.preferredWidth: root.width * 0.7
                Layout.fillHeight: true

                radius: 4
                border.width: tzContainer.activeFocus ? 3 : 1
                border.color: tzContainer.activeFocus ? LingmoUI.Theme.highlightColor : Qt.lighter(LingmoUI.Theme.backgroundColor, 1.5)
                color: LingmoUI.Theme.backgroundColor

                KeyNavigation.down: searchFieldFocusBox
                Keys.onReturnPressed: {
                    tzListView.forceActiveFocus()
                }

                onActiveFocusChanged: {
                    if(activeFocus) {
                        root.activeFocusedElement = objectName
                    }
                }

                LingmoUI.CardsListView {
                    id: tzListView
                    objectName: "tzListView"
                    anchors.fill: parent
                    anchors.margins: 8

                    spacing: 4
                    model: timezoneModule.model
                    clip: true
                    ScrollBar.vertical: ScrollBar {
                        active: true
                    }

                    delegate: LingmoUI.BasicListItem {
                        width: parent ? parent.width : 0
                        height: 40
                        label: tz

                        Keys.onReturnPressed: {
                            clicked()
                        }

                        Keys.onBackPressed: {
                            clicked()
                            tzContainer.forceActiveFocus()
                        }

                        Keys.onEscapePressed: {
                            clicked()
                            tzContainer.forceActiveFocus()
                        }

                        Keys.onSelectPressed: {
                            clicked()
                        }

                        onClicked: {
                            tzListView.currentIndex = index
                            nextButton.forceActiveFocus()
                        }
                    }

                    onActiveFocusChanged: {
                        if(activeFocus) {
                            root.activeFocusedElement = objectName
                        }
                    }

                    onCurrentIndexChanged: {
                        selectedZoneDisplayLabel.text = "Selected Timezone: " + tzListView.itemAtIndex(tzListView.currentIndex).text
                    }
                }
            }

            RowLayout {
                Layout.preferredWidth: root.width * 0.7
                Layout.preferredHeight: LingmoUI.Units.gridUnit * 3

                Rectangle {
                    id: searchFieldFocusBox
                    objectName: "searchFieldFocusBox"
                    Layout.fillWidth: true
                    Layout.preferredHeight: LingmoUI.Units.gridUnit * 3

                    color: "transparent"
                    border.color: searchFieldFocusBox.activeFocus ? LingmoUI.Theme.highlightColor : "transparent"
                    border.width: searchFieldFocusBox.activeFocus ? 3 : 0

                    KeyNavigation.down: nextButton
                    Keys.onReturnPressed: {
                        searchText.forceActiveFocus()
                    }

                    onActiveFocusChanged: {
                        if(activeFocus) {
                            root.activeFocusedElement = objectName
                        }
                    }

                    LingmoComponents.TextField {
                        id: searchText
                        anchors.fill: parent
                        anchors.margins: 5
                        topPadding: 16
                        bottomPadding: 16
                        placeholderText: qsTr("Search & Filter Timezone From List")

                        onTextChanged: {
                            timezoneModule.filterText = text
                            selectedZoneDisplayLabel.text = "Selected Timezone: " + tzListView.itemAtIndex(tzListView.currentIndex).text
                        }
                    }
                }

                Rectangle {
                    color: LingmoUI.Theme.backgroundColor
                    Layout.fillWidth: true
                    Layout.preferredHeight: LingmoUI.Units.gridUnit * 3


                    Label {
                        id: selectedZoneDisplayLabel
                        anchors.fill: parent
                        anchors.margins: LingmoUI.Units.largeSpacing
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        wrapMode: Text.WordWrap
                        text: "Selected Timezone: " + "Africa/Abidjan"
                        color: LingmoUI.Theme.textColor
                    }
                }
            }

            RowLayout {
                Layout.preferredWidth: root.width * 0.7
                Layout.preferredHeight: LingmoUI.Units.gridUnit * 3

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: LingmoUI.Units.gridUnit * 3
                    color: "transparent"
                    border.width: backButton.activeFocus ? 3 : 0
                    border.color: LingmoUI.Theme.highlightColor
                    radius: 3

                    Button {
                        id: backButton
                        anchors.fill: parent
                        anchors.margins: 2
                        highlighted: backButton.activeFocus ? 1 : 0
                        KeyNavigation.right: nextButton
                        KeyNavigation.up: searchFieldFocusBox

                        icon.name: "go-previous"
                        text: "Back"

                        Keys.onReturnPressed: clicked()

                        onClicked: {
                            moduleLoader.back()
                        }
                        visible: moduleLoader.hasPrevious
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: LingmoUI.Units.gridUnit * 3
                    color: "transparent"
                    border.width: nextButton.activeFocus ? 3 : 0
                    border.color: LingmoUI.Theme.highlightColor
                    radius: 3

                    NextButtonMediaCenter {
                        id: nextButton
                        objectName: "timezoneNextBtn"
                        anchors.fill: parent
                        anchors.margins: 2
                        highlighted: nextButton.activeFocus ? 1 : 0

                        KeyNavigation.left: backButton
                        KeyNavigation.right: skipButton
                        KeyNavigation.up: searchFieldFocusBox

                        onActiveFocusChanged: {
                            if(activeFocus) {
                                root.activeFocusedElement = objectName
                            }
                        }

                        onNextClicked: {
                            accepted = true
                            timezoneModule.setTimezone(tzListView.currentIndex)
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: LingmoUI.Units.gridUnit * 3
                    color: "transparent"
                    border.width: skipButton.activeFocus ? 3 : 0
                    border.color: LingmoUI.Theme.highlightColor
                    radius: 3

                    Button {
                        id: skipButton
                        objectName: "timezoneSkipButton"
                        anchors.fill: parent
                        anchors.margins: 2
                        highlighted: skipButton.activeFocus ? 1 : 0

                        KeyNavigation.left: nextButton
                        KeyNavigation.up: searchFieldFocusBox

                        icon.name: "go-next-skip"
                        text: "Skip"

                        visible: !hideSkip

                        Keys.onReturnPressed: clicked()

                        onClicked: {
                            moduleLoader.nextModule()
                        }

                        onActiveFocusChanged: {
                            if(activeFocus) {
                                root.activeFocusedElement = objectName
                            }
                        }
                    }
                }
            }
        }

        TimezoneModule {
            id: timezoneModule

            Component.onCompleted: {
                root.moduleName = "Timezone Setup"
                root.moduleIcon = timezoneModule.dir() + "/assets/timezone.svg"
            }

            property var signals: Connections {
                function onSetTimezoneSuccess() {
                    nextButton.next()
                }

                function onSetTimezoneFailed() {
                    nextButton.cancel()
                }

                function onErrorOccurred(err) {
                    console.log(qsTr("TZ ErrorOccurred : %1").arg(err))
                    applicationWindow().showPassiveNotification(err, 2000)
                }
            }
        }
    }
}
