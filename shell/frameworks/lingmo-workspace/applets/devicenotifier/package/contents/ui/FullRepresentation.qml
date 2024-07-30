/*
    SPDX-FileCopyrightText: 2011 Viranch Mehta <viranch.mehta@gmail.com>
    SPDX-FileCopyrightText: 2012 Jacopo De Simoi <wilderkde@gmail.com>
    SPDX-FileCopyrightText: 2014 David Edmundson <davidedmundson@kde.org>
    SPDX-FileCopyrightText: 2014 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2020 Nate Graham <nate@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Window
import QtQuick.Layouts

import org.kde.lingmo.plasmoid
import org.kde.lingmo.core as LingmoCore
import org.kde.lingmo.components as LingmoComponents3
import org.kde.lingmo.extras as LingmoExtras
import org.kde.lingmoui as LingmoUI

LingmoExtras.Representation {
    id: root
    readonly property var appletInterface: devicenotifier

    property bool spontaneousOpen: false

    property alias model: notifierDialog.model

    Layout.minimumWidth: LingmoUI.Units.gridUnit * 18
    Layout.minimumHeight: LingmoUI.Units.gridUnit * 18
    Layout.maximumWidth: LingmoUI.Units.gridUnit * 80
    Layout.maximumHeight: LingmoUI.Units.gridUnit * 40

    focus: true
    collapseMarginsHint: true

    header: LingmoExtras.PlasmoidHeading {
        visible: !(Plasmoid.containmentDisplayHints & LingmoCore.Types.ContainmentDrawsPlasmoidHeading) && devicenotifier.mountedRemovables > 1
        LingmoComponents3.ToolButton {
            id: unmountAll
            anchors.right: parent.right
            visible: devicenotifier.mountedRemovables > 1;

            icon.name: "media-eject"
            text: i18n("Remove All")
            Accessible.description: i18n("Click to safely remove all devices")

            LingmoComponents3.ToolTip {
                text: parent.Accessible.description
            }
        }
    }

    MouseArea {
        id: fullRepMouseArea
        hoverEnabled: true
    }

    // this item is reparented to a delegate that is showing a message to draw focus to it
    LingmoExtras.Highlight {
        id: messageHighlight
        visible: false

        OpacityAnimator {
            id: messageHighlightAnimator
            target: messageHighlight
            from: 1
            to: 0
            duration: LingmoUI.Units.veryLongDuration * 8
            easing.type: Easing.InOutQuad
            Component.onCompleted: devicenotifier.isMessageHighlightAnimatorRunning = Qt.binding(() => running);
        }

        Connections {
            target: filterModel
            function onLastUdiChanged() {
                if (filterModel.lastUdi === "") {
                    messageHighlightAnimator.stop()
                    messageHighlight.visible = false
                }
            }
        }

        function highlight(item) {
            parent = item
            width = Qt.binding(function() { return item.width })
            height = Qt.binding(function() { return item.height })
            opacity = 1 // Animator is threaded so the old opacity might be visible for a frame or two
            visible = true
            messageHighlightAnimator.start()
        }
    }

    LingmoComponents3.ScrollView {
        id: scrollView

        anchors.fill: parent
        contentWidth: availableWidth - contentItem.leftMargin - contentItem.rightMargin

        focus: true

        contentItem: ListView {
            id: notifierDialog
            focus: true

            model: filterModel

            delegate: DeviceItem {
            }

            highlight: LingmoExtras.Highlight
            {
            }
            highlightMoveDuration: LingmoUI.Units.shortDuration
            highlightResizeDuration: LingmoUI.Units.shortDuration

            topMargin: LingmoUI.Units.largeSpacing
            bottomMargin: LingmoUI.Units.largeSpacing
            leftMargin: LingmoUI.Units.largeSpacing
            rightMargin: LingmoUI.Units.largeSpacing
            spacing: LingmoUI.Units.smallSpacing

            currentIndex: devicenotifier.currentIndex

            //this is needed to make SectionScroller actually work
            //acceptable since one doesn't have a billion of devices
            cacheBuffer: 1000

            KeyNavigation.backtab: root.KeyNavigation.backtab
            KeyNavigation.up: root.KeyNavigation.up

            section {
                property: "deviceType"
                delegate: Item {
                    height: Math.floor(childrenRect.height)
                    width: notifierDialog.width - (scrollView.LingmoComponents3.ScrollBar.vertical.visible ? LingmoUI.Units.largeSpacing * 2 : 0)
                    LingmoUI.Heading {
                        level: 3
                        opacity: 0.6
                        text: section
                        textFormat: Text.PlainText
                    }
                }
            }

            Loader {
                anchors.centerIn: parent
                width: parent.width - (LingmoUI.Units.gridUnit * 4)

                active: notifierDialog.count === 0 && !messageHighlightAnimator.running
                visible: active
                asynchronous: true

                sourceComponent: LingmoExtras.PlaceholderMessage
                {
                    width: parent.width
                    iconName: "drive-removable-media-symbolic"
                    text: Plasmoid.configuration.removableDevices ? i18n("No removable devices attached") : i18n("No disks available")
                }
            }
        }
    }
}
