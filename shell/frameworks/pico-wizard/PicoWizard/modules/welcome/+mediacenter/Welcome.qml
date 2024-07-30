// SPDX-FileCopyrightText: 2021 Anupam Basak <anupam.basak27@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2021 Aditya Mehra <aix.m@outlook.com>
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.12 as Kirigami

import PicoWizard 1.0

FocusScope {
    id: welcomeQMLPage

    WelcomeModule {
        id: welcomeModule
    }

    Component.onCompleted: {
        parent.backgroundSource = Qt.resolvedUrl("../../assets/background-mediacenter.svg")
        welcomeQMLPage.forceActiveFocus()
    }

    KeyNavigation.down: nextBtn

    ColumnLayout {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.topMargin: parent.height * 0.15
        anchors.bottom: parent.bottom
        anchors.bottomMargin: parent.height * 0.05

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            Label {
                id: labelWelcome
                anchors.fill: parent
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                font.weight: Font.Light
                minimumPixelSize: 2
                font.pixelSize: 72
                maximumLineCount: 3
                fontSizeMode: Text.Fit
                wrapMode: Text.WordWrap
                text: qsTr("<font color=\"#1e88e5\"><b>Hello</b></font> Plasma Bigscreen")
                color: Kirigami.Theme.textColor
            }
        }

        Item {
            Layout.preferredWidth: Kirigami.Units.gridUnit * 18
            Layout.preferredHeight: Kirigami.Units.gridUnit * 6
            Layout.alignment: Qt.AlignHCenter

            Rectangle {
                color: Kirigami.Theme.highlightColor
                radius: 4
                width: Kirigami.Units.gridUnit * 6
                height: Kirigami.Units.gridUnit * 1
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
                    text: qsTr("Getting Started")
                    color: Kirigami.Theme.textColor
                }
            }

            Rectangle {
                id: infoRectContent
                color: Kirigami.Theme.backgroundColor
                radius: 4
                width: Kirigami.Units.gridUnit * 17.5
                height: Kirigami.Units.gridUnit * 5
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottom: parent.bottom
                anchors.bottomMargin: Kirigami.Units.smallSpacing
                z: 1

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: Kirigami.Units.largeSpacing

                    RowLayout {
                        Layout.fillWidth: true
                        Layout.fillHeight: true

                        Item {
                            Layout.preferredWidth: Kirigami.Units.gridUnit * 2
                            Layout.fillHeight: true

                            Kirigami.Icon {
                                anchors.fill: parent
                                source: welcomeModule.dir() + "/assets/remote-ok.svg"
                            }
                        }

                        Kirigami.Separator {
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
                            text: qsTr('Remote: Press the "Select|OK" button to continue')
                            color: Kirigami.Theme.textColor
                        }
                    }

                    Kirigami.Separator {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 1
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        Layout.fillHeight: true

                        Item {
                            Layout.preferredWidth: Kirigami.Units.gridUnit * 2
                            Layout.fillHeight: true

                            Kirigami.Icon {
                                anchors.fill: parent
                                source: welcomeModule.dir() + "/assets/keyboard-ok.svg"
                            }
                        }

                        Kirigami.Separator {
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
                            text: qsTr('Keyboard: Press the "Enter" button to continue')
                            color: Kirigami.Theme.textColor
                        }
                    }
                }
            }
        }

        Item {
            Layout.fillWidth: true
            Layout.preferredHeight: parent.height * 0.25
            Layout.alignment: Qt.AlignHCenter

            Button {
                id: nextBtn
                focus: true
                width: Kirigami.Units.gridUnit * 7 //parent.height * 0.6
                height: parent.height * 0.6
                anchors.centerIn: parent

                background: Rectangle {
                    color: nextBtn.activeFocus ? Kirigami.Theme.highlightColor : Qt.lighter(Kirigami.Theme.backgroundColor, 1.2)
                    border.color: nextBtn.activeFocus ? Kirigami.Theme.highlightColor : Qt.lighter(Kirigami.Theme.backgroundColor, 1.2)
                    border.width: nextBtn.activeFocus ? 4 : 0
                    radius: 4
                }

                contentItem: Item {
                    anchors.fill: parent
                    anchors.margins: Kirigami.Units.smallSpacing

                    Item {
                        id: nextBtnTextItem
                        anchors.left: parent.left
                        width: parent.width * 0.75
                        height: parent.height

                        Label {
                            anchors.fill: parent
                            horizontalAlignment: Text.AlignRight
                            verticalAlignment: Text.AlignVCenter
                            font.pixelSize: parent.width * 0.22
                            wrapMode: Text.WordWrap
                            text: qsTr("Continue")
                            color: Kirigami.Theme.textColor
                        }
                    }
                    Item {
                        id: nextBtnIconItem
                        anchors.left: nextBtnTextItem.right
                        width: parent.width * 0.25
                        height: parent.height

                        Kirigami.Icon {
                            id: nextIcon
                            anchors.fill: parent
                            anchors.margins: -Kirigami.Units.gridUnit
                            color: Kirigami.Theme.highlightedTextColor
                            isMask: true
                            source: welcomeModule.dir() + "/assets/next.svg"
                        }
                    }
                }

                Keys.onReturnPressed: {
                    moduleLoader.nextModule()
                }

                onClicked: {
                    moduleLoader.nextModule()
                }
            }
        }
    }
}
