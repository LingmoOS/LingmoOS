// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2021 Aditya Mehra <aix.m@outlook.com>
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import org.kde.plasma.components 3.0 as PlasmaComponents
import org.kde.kirigami 2.9 as Kirigami

import PicoWizard 1.0

ModuleMediaCenter {
    id: root
    property bool mycroftEnabled: true

    delegate: Item {
        id: delegateItem

        Component.onCompleted: {
            enableMycroftBox.forceActiveFocus()
        }

        ColumnLayout {
            width: parent.parent.parent.width
            anchors {
                top: parent.top
                bottom: parent.bottom
                bottomMargin: 16
            }

            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true

                Rectangle {
                    id: enableMycroftBox
                    anchors.left: parent.left
                    width: parent.width / 2 - Kirigami.Units.smallSpacing
                    height: parent.height
                    Kirigami.Theme.colorSet: Kirigami.Theme.Button
                    color: Kirigami.Theme.backgroundColor
                    radius: 3
                    border.color: enableMycroftBox.activeFocus ? Kirigami.Theme.highlightColor : Kirigami.Theme.disabledTextColor
                    border.width: enableMycroftBox.activeFocus ? 3 : 1

                    KeyNavigation.right: disableMycroftBox
                    KeyNavigation.down: nextButton

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: Kirigami.Units.largeSpacing

                        Kirigami.Icon {
                            source: mycroftModule.dir() + "/assets/tv.svg"
                            Layout.fillWidth: true
                            Layout.preferredHeight: parent.height / 4.5
                            color: "white"
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            Layout.preferredHeight: parent.height / 4.5

                            Kirigami.Icon {
                                source: mycroftModule.dir() + "/assets/remote.svg"
                                Layout.fillWidth: true
                                Layout.preferredHeight: remIcn.height
                                color: "white"
                            }
                            Kirigami.Icon {
                                source: mycroftModule.dir() + "/assets/micarray.svg"
                                Layout.fillWidth: true
                                Layout.preferredHeight: remIcn.height
                                color: "white"
                            }
                        }

                        Item {
                            Layout.fillWidth: true
                            Layout.fillHeight: true

                            Label {
                                anchors.fill: parent
                                wrapMode: Text.WordWrap
                                color: "white"
                                minimumPixelSize: height * 0.05
                                horizontalAlignment: Text.AlignHCenter
                                fontSizeMode: Text.Fit
                                font.pixelSize: height * 0.20
                                text: "Enable Mycroft Integration"
                            }
                        }

                        Kirigami.Icon {
                            source: mycroftModule.dir() + "/assets/greentick.svg"
                            visible: mycroftEnabled
                            Layout.fillWidth: true
                            Layout.preferredHeight: parent.height * 0.10
                        }
                    }


                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            root.mycroftEnabled = true
                        }
                    }
                    Keys.onReturnPressed: {
                        root.mycroftEnabled = true
                    }
                }
                Rectangle {
                    id: disableMycroftBox
                    anchors.right: parent.right
                    width: parent.width / 2 - Kirigami.Units.smallSpacing
                    height: parent.height
                    Kirigami.Theme.colorSet: Kirigami.Theme.Button
                    color: Kirigami.Theme.backgroundColor
                    radius: 3
                    border.color: disableMycroftBox.activeFocus ? Kirigami.Theme.highlightColor : Kirigami.Theme.disabledTextColor
                    border.width: disableMycroftBox.activeFocus ? 3 : 1

                    KeyNavigation.left: enableMycroftBox
                    KeyNavigation.down: nextButton

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: Kirigami.Units.largeSpacing

                        Kirigami.Icon {
                            source: mycroftModule.dir() + "/assets/tv.svg"
                            Layout.fillWidth: true
                            Layout.preferredHeight: parent.height / 4.5
                            color: "white"
                        }

                        Kirigami.Icon {
                            id: remIcn
                            source: mycroftModule.dir() + "/assets/remote.svg"
                            Layout.fillWidth: true
                            Layout.preferredHeight: parent.height / 4.5
                            color: "white"
                        }

                        Item {
                            Layout.fillWidth: true
                            Layout.fillHeight: true

                            Label {
                                anchors.fill: parent
                                wrapMode: Text.WordWrap
                                color: "white"
                                minimumPixelSize: height * 0.05
                                horizontalAlignment: Text.AlignHCenter
                                fontSizeMode: Text.Fit
                                font.pixelSize: height * 0.20
                                text: "Disable Mycroft Integration"
                            }
                        }

                        Kirigami.Icon {
                            source: mycroftModule.dir() + "/assets/greentick.svg"
                            visible: !mycroftEnabled
                            Layout.fillWidth: true
                            Layout.preferredHeight: parent.height * 0.10
                        }
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            root.mycroftEnabled = false
                        }
                    }
                    Keys.onReturnPressed: {
                        root.mycroftEnabled = false
                    }
                }
            }

            Kirigami.Separator {
                Layout.fillWidth: true
                Layout.preferredHeight: 1
            }

            Rectangle {
                id: mycroftExplainBox
                color: "transparent"
                Layout.fillWidth: true
                Layout.preferredHeight: Kirigami.Units.gridUnit * 4

                Loader {
                    id: textLoader
                    anchors.fill: parent
                    sourceComponent: mycroftEnabled ? enabledTextComp : disabledTextComp
                }
            }

            RowLayout {
                Layout.preferredWidth: root.width * 0.7
                Layout.preferredHeight: Kirigami.Units.gridUnit * 3

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: Kirigami.Units.gridUnit * 3
                    color: "transparent"
                    border.width: backButton.activeFocus ? 3 : 0
                    border.color: Kirigami.Theme.highlightColor
                    radius: 3

                    Button {
                        id: backButton
                        anchors.fill: parent
                        anchors.margins: 2
                        highlighted: backButton.activeFocus ? 1 : 0
                        KeyNavigation.right: nextButton
                        KeyNavigation.up: enableMycroftBox

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
                    Layout.preferredHeight: Kirigami.Units.gridUnit * 3
                    color: "transparent"
                    border.width: nextButton.activeFocus ? 3 : 0
                    border.color: Kirigami.Theme.highlightColor
                    radius: 3

                    NextButtonMediaCenter {
                        id: nextButton
                        anchors.fill: parent
                        anchors.margins: 2
                        highlighted: nextButton.activeFocus ? 1 : 0

                        KeyNavigation.left: backButton
                        KeyNavigation.up: enableMycroftBox

                        onNextClicked: {
                            accepted = true
                            if(mycroftEnabled) {
                                mycroftModule.enableMycroftIntegration()
                            } else {
                                mycroftModule.disableMycroftIntegration()
                            }
                        }
                    }
                }
            }
        }

        MycroftModule {
            id: mycroftModule

            Component.onCompleted: {
                root.moduleName = mycroftModule.moduleName() + " Integration Settings"
                root.moduleIcon = mycroftModule.dir() + "/assets/mic.svg"
            }

            property var signals: Connections {
                function onMycroftEnabledSuccess() {
                    nextButton.next()
                }
                function onMycroftDisabledSuccess() {
                    nextButton.next()
                }

                function onMycroftEnabledFailed() {
                    nextButton.cancel()
                }

                function onMycroftDisabledFailed(err) {
                    applicationWindow().showPassiveNotification(err, 2000)
                }
            }
        }

        Component {
            id: enabledTextComp

            ColumnLayout {
                width: parent.width
                height: parent.height
                spacing: Kirigami.Units.largeSpacing

                Label {
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                    text: "Mycroft offers voice commands, voice searching, and voice-activated device control, letting you complete a number of tasks after you've said the 'Hey Mycroft' wake word.\nMycroft Integration is optional and can always be turned off in Bigscreen Settings Application, which will disable mycroft starting on boot and disable the voice application section"
                }
            }
        }

        Component {
            id: disabledTextComp
            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Label {
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    elide: Text.ElideRight
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    text: "Mycroft Integration will be disabled by default, It can easily be enabled in Bigscreen Settings Application"
                }
            }
        }
    }
}
