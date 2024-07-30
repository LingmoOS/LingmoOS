/*
 *  SPDX-FileCopyrightText: 2022 Nate Graham <nate@kde.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Effects

import org.kde.kirigami as Kirigami
import org.kde.plasma.components as PC3

import org.kde.plasma.welcome

GenericPage {
    id: root

    heading: i18nc("@info:window", "Access the Internet")
    description: xi18nc("@info:usagetip", "You can connect to the internet and manage your network connections with the <interface>Networks applet</interface>. To access it, click on the <interface>Networks</interface> icon in your <interface>System Tray</interface>, which lives in the bottom-right corner of the screen.")

    Loader {
        id: nmLoader
        source: "PlasmaNM.qml"

        // Defaults for when PlasmaNM is not available
        property bool statusConnected: false
        property bool iconConnecting: false
        property string icon: "network-wireless-available"

        onStatusChanged: {
            if (status === Loader.Ready) {
                statusConnected = Qt.binding(() => nmLoader.item.networkStatus.connectivity === 4); // 4, Full connectivity
                iconConnecting = Qt.binding(() => nmLoader.item.connectionIcon.connecting);
                icon = Qt.binding(() => nmLoader.item.connectionIcon.connectionIcon);
            } else if (status === Loader.Error) {
                console.warn("PlasmaNM integration failed to load (is plasma-nm available?)");
            }
        }

        // Continue to the next page automatically when connected
        onStatusConnectedChanged: {
            if (statusConnected && pageStack.currentItem == root) {
                pageStack.currentIndex += 1;
            }
        }
    }

    states: [
        State {
            name: "NoPlasmaNM" // Shows error message
            when: nmLoader.status == Loader.Error
        },
        State {
            name: "Connected" // Shows card and connected message
            when: nmLoader.statusConnected
        },
        State {
            name: "Disconnected" // Shows card
            when: !nmLoader.statusConnected
        }
    ]

    Kirigami.PlaceholderMessage {
        id: errorMessage

        anchors.centerIn: parent
        width: parent.width - Kirigami.Units.gridUnit * 2

        // Shown when PlasmaNM is not available
        visible: root.state == "NoPlasmaNM"

        icon.name: "data-warning-symbolic"
        text: i18nc("@info:placeholder", "Networking support for Plasma is not currently installed")
        explanation: xi18nc("@info:usagetip %1 is the name of the user's distro", "To manage network connections, Plasma requires <icode>plasma-nm</icode> to be installed. Please report this omission to %1.", Controller.distroName())
        helpfulAction: Kirigami.Action {
            enabled: root.state == "NoPlasmaNM"
            icon.name: "tools-report-bug-symbolic"
            text: i18nc("@action:button", "Report Bug…")
            onTriggered: Qt.openUrlExternally(Controller.distroBugReportUrl())
        }
    }

    Kirigami.AbstractCard {
        anchors.fill: parent

        // Shown when the user is connected or disconnected
        visible: (root.state == "Connected" || root.state == "Disconnected") ? 1 : 0

        MockDesktop {
            id: mockDesktop
            anchors.fill: parent
            anchors.margins: Kirigami.Units.smallSpacing

            opacity: root.state == "Connected" ? 0.6 : 1
            layer.enabled: true

            Behavior on opacity {
                NumberAnimation {
                    duration: Kirigami.Units.longDuration
                    easing.type: Easing.InOutQuad
                }
            }

            backgroundAlignment: Qt.AlignRight | Qt.AlignBottom

            MockPanel {
                id: mockPanel
                anchors.bottom: parent.bottom
                anchors.right: parent.right

                width: Math.max(mockDesktop.desktopWidth, mockDesktop.width)

                MockKickoffApplet {
                    opacity: (mockPanel.x >= 0) ? 1 : 0
                    visible: opacity > 0

                    Behavior on opacity {
                        NumberAnimation {
                            duration: Kirigami.Units.longDuration
                            easing.type: Easing.InOutQuad
                        }
                    }
                }

                Item {
                    Layout.fillWidth: true
                }

                MockSystemTrayApplet {
                    id: mockSystemTray

                    MockSystemTrayIcon {
                        source: "klipper-symbolic"
                    }

                    MockSystemTrayIcon {
                        source: "audio-volume-high-symbolic"
                    }

                    MockSystemTrayIcon {
                        id: mockNmTrayIcon

                        source: nmLoader.icon

                        PC3.BusyIndicator {
                            anchors.centerIn: parent

                            // Yes, it's not square
                            width: 30
                            height: 28

                            running: nmLoader.iconConnecting
                        }

                        Item {
                            id: indicatorArrowContainer

                            readonly property bool animate: visible && root.state !== "Connected" && pageStack.currentItem == root

                            implicitWidth: indicatorArrow.implicitWidth + glowPadding * 2
                            implicitHeight: indicatorArrow.implicitHeight + glowPadding * 2

                            // Prevents clipping of the glow effect
                            property real glowPadding: 16

                            property real yOffset: Kirigami.Units.gridUnit

                            anchors.horizontalCenter: parent.horizontalCenter
                            anchors.bottom: parent.top
                            anchors.bottomMargin: yOffset - glowPadding
                                                  + ((mockSystemTray.height - mockNmTrayIcon.height) / 2)

                            SequentialAnimation on yOffset {
                                running: indicatorArrowContainer.animate
                                loops: Animation.Infinite
                                alwaysRunToEnd: true

                                NumberAnimation {
                                    from: Kirigami.Units.gridUnit
                                    to: 0
                                    duration: 1000
                                    easing.type: Easing.InOutQuad
                                }
                                NumberAnimation {
                                    from: 0
                                    to: Kirigami.Units.gridUnit
                                    duration: 1000
                                    easing.type: Easing.InOutQuad
                                }
                            }

                            layer.enabled: true
                            opacity: animate ? 1 : 0

                            Behavior on opacity {
                                NumberAnimation {
                                    duration: Kirigami.Units.longDuration
                                    easing.type: Easing.InOutQuad
                                }
                            }

                            MultiEffect {
                                anchors.fill: indicatorArrow

                                autoPaddingEnabled: false
                                paddingRect: Qt.rect(indicatorArrowContainer.glowPadding,
                                                    indicatorArrowContainer.glowPadding,
                                                    indicatorArrowContainer.glowPadding,
                                                    indicatorArrowContainer.glowPadding)

                                source: indicatorArrow
                                blurEnabled: true
                                blur: 0.25
                                colorization: 1.0
                                colorizationColor: "white"
                                brightness: 2.0
                            }

                            Kirigami.Icon {
                                id: indicatorArrow
                                anchors.centerIn: indicatorArrowContainer

                                implicitWidth: Kirigami.Units.iconSizes.large * 1.5
                                implicitHeight: Kirigami.Units.iconSizes.large * 1.5

                                source: "arrow-down"
                            }
                        }
                    }
                }

                MockDigitalClockApplet {}

                MockShowDesktopApplet {}
            }
        }

        Kirigami.PlaceholderMessage {
            id: connectedMessage

            anchors.centerIn: mockDesktop
            width: mockDesktop.width - Kirigami.Units.gridUnit * 2

            // Shown when connected
            visible: root.state == "Connected"

            icon.name: "data-success-symbolic"
            text: i18nc("@info:placeholder Shown when connected to the internet", "You’re connected")
            explanation: i18nc("@info:usagetip Shown when connected to the internet", "All good to go!")
            type: Kirigami.PlaceholderMessage.Type.Actionable
        }
    }

    footer: Kirigami.InlineMessage {
        position: Kirigami.InlineMessage.Footer
        visible: Controller.patchVersion === 80
        text: i18nc("@info", "This page is being shown regardless of network connectivity because you are using a development version. To manually preview the different states of the page, you can use the button.")
        showCloseButton: true

        actions: [
            Kirigami.Action {
                text: i18nc("@action:button", "Change state")
                onTriggered: {
                    let stateIndex = root.states.findIndex(state => state.name == root.state)
                    root.state = root.states[(stateIndex + 1) % root.states.length].name
                }
            }
        ]
    }
}
