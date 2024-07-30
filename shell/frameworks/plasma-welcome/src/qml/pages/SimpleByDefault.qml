/*
 *  SPDX-FileCopyrightText: 2024 Nate Graham <nate@kde.org>
 *  SPDX-FileCopyrightText: 2024 Oliver Beard <olib141@outlook.com
 *
 *  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.ksvg as KSvg
import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.components as PC3

import org.kde.plasma.welcome

GenericPage {
    id: root

    heading: i18nc("@info:window", "Simple by Default")
    description: xi18nc("@info:usagetip", "Plasma is designed to be simple and usable out of the box. Things are where you would expect, and there is generally no need to configure anything before you can be comfortable and productive.<nl/><nl/>Below is a visual representation of a typical Plasma Desktop; move the pointer over or click on something to learn about it!")

    property var activeItem: null

    // NOTE: Without setting accepted devices, a unhover event is sent on tap with a touchscreen, breaking the interaction.
    // This is set for all HoverHandlers.
    property var hoverHandlerAcceptedDevices: PointerDevice.Mouse | PointerDevice.TouchPad | PointerDevice.Stylus

    onActiveItemChanged: {
        switch (activeItem) {
            case mockDesktop:
                explanatoryLabel.text = xi18nc("@info", "This is the “Desktop”. It shows files and folders that are contained in your <filename>Desktop</filename> folder, and can hold widgets. Right-click on it and choose <interface>Desktop and Wallpaper…</interface> to configure the appearance of the desktop. You can also choose <interface>Enter Edit Mode</interface> to add, remove or modify widgets.")
                break;
            case mockPanel:
                explanatoryLabel.text = xi18nc("@info", "This is a “Panel” — a container to hold widgets. Right-click on it and choose <interface>Show Panel Configuration</interface> to change how it behaves, which screen edge it lives on, and to add, remove or modify widgets.");
                break;
            case mockKickoff:
                explanatoryLabel.text = i18nc("@info", "This is the “Kickoff” widget, a multipurpose launcher. Here you can launch apps, shut down or restart the system, access recent files, and more. Click on it to get started!");
                break;
            case mockTaskManager:
                explanatoryLabel.text = i18nc("@info", "This is the “Task Manager” widget, where you can switch between open apps and also launch new ones. Drag app icons to re-arrange them.");
                break;
            case mockTray:
                explanatoryLabel.text = i18nc("@info", "This is the “System Tray” widget, which lets you control system functions like changing the volume and connecting to networks. Items will become visible here only when relevant. To see all available items, click the ⌃ arrow next to the clock.");
                break;
            case mockClock:
                explanatoryLabel.text = i18nc("@info", "This is the “Digital Clock” widget. Click on it to show a calendar. It can be also configured to show other timezones and events from your digital calendars.");
                break;
            case mockShowDesktop:
                explanatoryLabel.text = i18nc("@info", "This is the “Peek at Desktop” widget. Click on it to temporarily hide all open windows so you can access the desktop. Click on it again to bring them back.");
                break;
            case null:
            default:
                // We don't set the text to empty so that the explanation doesn't change height whilst animating away.
                break;
        }
    }

    function handleHovered(item, hovered) {
        if (mockPanel.overflowing) {
            return;
        }

        if (hovered) {
            activeItem = item;
        } else if (!hovered && activeItem == item) {
            activeItem = null
        }
    }

    function handleTapped(item) {
        if (mockPanel.overflowing) {
            return;
        }

        if (activeItem != item) {
            activeItem = item;
        }
    }

    Kirigami.AbstractCard {
        anchors.fill: parent

        MockDesktop {
            id: mockDesktop
            anchors.fill: parent
            anchors.margins: Kirigami.Units.smallSpacing

            backgroundAlignment: Qt.AlignHCenter | Qt.AlignBottom

            MockPanel {
                id: mockPanel
                anchors.bottom: parent.bottom
                anchors.right: parent.right

                // Bizarrely, using width directly causes a binding loop, so we must
                // use mockDesktop.width (which will always be the width)
                readonly property bool overflowing: mockDesktop.width < implicitWidth

                width: mockDesktop.width

                MockKickoffApplet {
                    id: mockKickoff

                    opacity: mockPanel.overflowing ? 0 : 1
                    active: root.activeItem == mockKickoff
                    Behavior on opacity { NumberAnimation { duration: Kirigami.Units.longDuration; easing.type: Easing.InOutQuad }}

                    HoverHandler { acceptedDevices: root.hoverHandlerAcceptedDevices
                                   onHoveredChanged: root.handleHovered(mockKickoff, hovered) }
                    TapHandler { onTapped: root.handleTapped(mockKickoff) }
                }

                MockTaskManager {
                    id: mockTaskManager

                    opacity: mockPanel.overflowing ? 0 : 1
                    active: root.activeItem == mockTaskManager
                    Behavior on opacity { NumberAnimation { duration: Kirigami.Units.longDuration; easing.type: Easing.InOutQuad }}

                    HoverHandler { acceptedDevices: root.hoverHandlerAcceptedDevices
                                   onHoveredChanged: root.handleHovered(mockTaskManager, hovered) }
                    TapHandler { onTapped: root.handleTapped(mockTaskManager) }
                }

                // Handlers for panel, with a minimum free space
                Item {
                    Layout.preferredWidth: Kirigami.Units.gridUnit * 4
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    HoverHandler { acceptedDevices: root.hoverHandlerAcceptedDevices
                                   onHoveredChanged: root.handleHovered(mockPanel, hovered) }
                    TapHandler { onTapped: root.handleTapped(mockPanel) }
                }

                // We have to wrap the tray to add a hover/tap handler
                // as children get added to the panel's layout
                Item {
                    width: mockTray.width
                    height: mockTray.height

                    opacity: mockPanel.overflowing ? 0 : 1
                    Behavior on opacity { NumberAnimation { duration: Kirigami.Units.longDuration; easing.type: Easing.InOutQuad }}

                    MockSystemTrayApplet {
                        id: mockTray

                        active: root.activeItem == mockTray

                        MockSystemTrayIcon { source: "klipper-symbolic" }
                        MockSystemTrayIcon { source: "audio-volume-high-symbolic" }
                    }

                    HoverHandler { acceptedDevices: root.hoverHandlerAcceptedDevices
                                   onHoveredChanged: root.handleHovered(mockTray, hovered) }
                    TapHandler { onTapped: root.handleTapped(mockTray) }
                }

                MockDigitalClockApplet {
                    id: mockClock

                    opacity: mockPanel.overflowing ? 0 : 1
                    active: root.activeItem == mockClock
                    Behavior on opacity { NumberAnimation { duration: Kirigami.Units.longDuration; easing.type: Easing.InOutQuad }}

                    HoverHandler { acceptedDevices: root.hoverHandlerAcceptedDevices
                                   onHoveredChanged: root.handleHovered(mockClock, hovered) }
                    TapHandler { onTapped: root.handleTapped(mockClock) }
                }

                MockShowDesktopApplet {
                    id: mockShowDesktop

                    opacity: mockPanel.overflowing ? 0 : 1
                    active: root.activeItem == mockShowDesktop
                    Behavior on opacity { NumberAnimation { duration: Kirigami.Units.longDuration; easing.type: Easing.InOutQuad }}

                    HoverHandler { acceptedDevices: root.hoverHandlerAcceptedDevices
                                   onHoveredChanged: root.handleHovered(mockShowDesktop, hovered) }
                    TapHandler { onTapped: root.handleTapped(mockShowDesktop) }
                }
            }

            PC3.Label {
                anchors.centerIn: mockPanel

                opacity: mockPanel.overflowing ? 1 : 0
                visible: opacity > 0

                Behavior on opacity {
                    NumberAnimation {
                        duration: Kirigami.Units.longDuration
                        easing.type: Easing.InOutQuad
                    }
                }

                text: i18nc("@info:placeholder Shown when there is insufficent width", "Expand the window")
                color: PlasmaCore.Theme.textColor
            }

            Item {
                id: explanatoryContainer

                anchors.centerIn: parent
                anchors.verticalCenterOffset: Math.round(-mockPanel.height / 2)

                width: explanatoryShadow.width
                height: explanatoryShadow.height

                layer.enabled: true
                opacity: (root.activeItem == null || mockPanel.overflowing) ? 0 : 1
                visible: opacity > 0

                Behavior on opacity {
                    NumberAnimation {
                        duration: Kirigami.Units.longDuration
                        easing.type: Easing.InOutQuad
                    }
                }

                KSvg.FrameSvgItem {
                    id: explanatoryShadow
                    anchors.fill: explanatoryBackground

                    anchors.topMargin: -margins.top
                    anchors.leftMargin: -margins.left
                    anchors.rightMargin: -margins.right
                    anchors.bottomMargin: -margins.bottom

                    imagePath: "widgets/tooltip"
                    prefix: "shadow"
                }

                KSvg.FrameSvgItem {
                    id: explanatoryBackground
                    anchors.fill: explanatoryTextContainer

                    anchors.topMargin: -margins.top
                    anchors.leftMargin: -margins.left
                    anchors.rightMargin: -margins.right
                    anchors.bottomMargin: -margins.bottom

                    imagePath: "widgets/tooltip"
                }

                // We have to wrap the label so we can clip the text
                Item {
                    id: explanatoryTextContainer
                    anchors.centerIn: parent

                    width: Math.min(Math.round(mockDesktop.width / 1.5), Kirigami.Units.gridUnit * 25) + explanatoryLabel.anchors.margins * 2
                    height: explanatoryLabel.implicitHeight + explanatoryLabel.anchors.margins * 2

                    Behavior on height {
                        NumberAnimation {
                            // Don't animate the height if we aren't visible!
                            duration: explanatoryContainer.visible ? Kirigami.Units.shortDuration : 0
                            easing.type: Easing.InOutQuad
                        }
                    }

                    clip: true

                    PC3.Label {
                        id: explanatoryLabel
                        anchors.margins: Kirigami.Units.largeSpacing
                        anchors.left: parent.left
                        anchors.top: parent.top
                        anchors.right: parent.right

                        wrapMode: Text.Wrap

                        color: PlasmaCore.Theme.textColor
                    }
                }
            }

            // Handlers for desktop
            Item {
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: mockPanel.top

                HoverHandler { acceptedDevices: root.hoverHandlerAcceptedDevices
                               onHoveredChanged: root.handleHovered(mockDesktop, hovered) }
                TapHandler { onTapped: root.handleTapped(mockDesktop) }
            }
        }
    }
}
