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

    delegate: Item {
        id: delegateItem
        property int setupStage: 0
        property int keyStage: 0

        Component.onCompleted: {
            setupCecBox.forceActiveFocus()
            cecModule.initilize_cec()
        }

        onKeyStageChanged: {
            stageLoader.keyStage = keyStage
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
                    id: setupCecBox
                    anchors.left: parent.left
                    width: parent.width - Kirigami.Units.smallSpacing
                    height: parent.height
                    Kirigami.Theme.colorSet: Kirigami.Theme.Button
                    color: Kirigami.Theme.backgroundColor
                    radius: 3
                    border.color: setupCecBox.activeFocus ? Kirigami.Theme.highlightColor : Kirigami.Theme.disabledTextColor
                    border.width: setupCecBox.activeFocus ? 3 : 1

                    KeyNavigation.down: nextButton.enabled ? nextButton : backButton
                    
                    Loader {
                        id: stageLoader
                        anchors.fill: parent
                        property int keyStage: delegateItem.keyStage
                        sourceComponent: switch(delegateItem.setupStage) {
                            case 0: return loadingComponent;
                            case 1: return foundCecComponent;
                            case 2: return notFoundCecComponent;
                            case 3: return keyMapperComponent;
                            case 4: return finishComponentCecReady;
                        }
                    }
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
                        KeyNavigation.right: restartButton.visible ? restartButton : nextButton
                        KeyNavigation.up: setupCecBox

                        icon.name: "go-previous"
                        text: "Back"

                        Keys.onReturnPressed: clicked()

                        onClicked: {
                            cecModule.close_cec_thread()
                            moduleLoader.back()
                        }
                        visible: moduleLoader.hasPrevious
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: Kirigami.Units.gridUnit * 3
                    color: "transparent"
                    border.width: restartButton.activeFocus ? 3 : 0
                    border.color: Kirigami.Theme.highlightColor
                    radius: 3
                    visible: delegateItem.setupStage == 3
                    enabled: delegateItem.setupStage == 3

                    Button {
                        id: restartButton
                        anchors.fill: parent
                        anchors.margins: 2
                        highlighted: restartButton.activeFocus ? 1 : 0
                        KeyNavigation.left: backButton
                        KeyNavigation.right: nextButton.enabled ? nextButton : skipButton
                        KeyNavigation.up: setupCecBox

                        icon.name: "view-refresh"
                        text: "Restart"

                        Keys.onReturnPressed: clicked()

                        onClicked: {
                            cecModule.restart_cec_mapping()
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: Kirigami.Units.gridUnit * 3
                    color: "transparent"
                    border.width: nextButton.activeFocus ? 3 : 0
                    border.color: Kirigami.Theme.highlightColor
                    enabled: delegateItem.setupStage == 2 || delegateItem.setupStage == 4
                    radius: 3

                    NextButtonMediaCenter {
                        id: nextButton
                        anchors.fill: parent
                        anchors.margins: 2
                        enabled: delegateItem.setupStage == 2 || delegateItem.setupStage == 4
                        highlighted: nextButton.activeFocus ? 1 : 0

                        KeyNavigation.left: restartButton.visible ? restartButton : backButton
                        KeyNavigation.up: setupCecBox

                        onNextClicked: {
                            cecModule.close_cec_thread()
                            accepted = true
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: Kirigami.Units.gridUnit * 3
                    color: "transparent"
                    border.width: skipButton.activeFocus ? 3 : 0
                    border.color: Kirigami.Theme.highlightColor
                    radius: 3

                    Button {
                        id: skipButton
                        anchors.fill: parent
                        anchors.margins: 2
                        highlighted: skipButton.activeFocus ? 1 : 0

                        KeyNavigation.left: nextButton.enabled ? nextButton : restartButton.visible ? restartButton : backButton
                        KeyNavigation.up: setupCecBox

                        icon.name: "go-next-skip"
                        text: "Skip"

                        visible: !hideSkip

                        Keys.onReturnPressed: clicked()

                        onClicked: {
                            cecModule.end_cec_mapping_success()
                            cecModule.close_cec_thread()
                            moduleLoader.nextModule()
                        }
                    }
                }
            }
        }

        CecModule {
            id: cecModule

            Component.onCompleted: {
                root.moduleName = "HDMI-CEC Setup"
                root.moduleIcon = cecModule.dir() + "/assets/remote-control.svg"
            }

            property var signals: Connections {
                function onSetupStageChanged() {
                    delegateItem.setupStage = cecModule.getCurrentSetupStage()
                }

                function onKeyStageChanged() {
                    delegateItem.keyStage = cecModule.getCurrentKeyStage()
                }
            }
        }

        Component {
            id: loadingComponent

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
                    text: "Loading... Please wait. Checking for CEC devices..."
                }
            }
        }

        Component {
            id: foundCecComponent

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
                    text: "CEC device found. Starting CEC remote control setup..."
                }
            }
        }

        Component {
            id: notFoundCecComponent

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
                    text: "CEC device not found. Click next to continue."
                }
            }
        }

        Component {
            id: keyMapperComponent
            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Kirigami.Heading {
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    elide: Text.ElideRight
                    level: 2
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    text: "Press the key on your remote to map it to the following actions"
                }

                RowLayout {
                    id: remoteKeyPressStageLayout
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.alignment: Qt.AlignTop | Qt.AlignHCenter

                    property var stageKey: switch(stageLoader.keyStage) {
                        case 0: return "Left";
                        case 1: return "Right";
                        case 2: return "Up";
                        case 3: return "Down";
                        case 4: return "Ok";
                        case 5: return "Back";
                        case 6: return "Home";
                    }

                    Kirigami.Heading {
                        wrapMode: Text.WordWrap
                        Layout.preferredWidth: parent.width / 2
                        Layout.fillHeight: true
                        elide: Text.ElideRight
                        level: 3
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        text: "Select / Press the " + remoteKeyPressStageLayout.stageKey + " key on your remote"
                    }

                    Image {
                        id: remoteKeyPressStageIcon
                        source: switch(stageLoader.keyStage) {
                            case 0: return cecModule.dir() + "/assets/remote-left.svg";
                            case 1: return cecModule.dir() + "/assets/remote-right.svg";
                            case 2: return cecModule.dir() + "/assets/remote-up.svg";
                            case 3: return cecModule.dir() + "/assets/remote-down.svg";
                            case 4: return cecModule.dir() + "/assets/remote-ok.svg";
                            case 5: return cecModule.dir() + "/assets/remote-back.svg";
                            case 6: return cecModule.dir() + "/assets/remote-home.svg";
                        }
                        Layout.preferredWidth: 380
                        Layout.fillHeight: true
                    }
                }
            }
        }

        Component {
            id: finishComponentCecReady
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
                    text: "CEC remote control setup complete! You can now use your remote to control your TV. Click next to continue."
                }
            }
        }
    }
}
