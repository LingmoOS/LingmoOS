/*
    SPDX-FileCopyrightText: 2012 Aleix Pol Gonzalez <aleixpol@blue-systems.com>
    SPDX-FileCopyrightText: 2019 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

Flickable {
    id: root

    property alias screenshotsModel: screenshotsRep.model
    readonly property alias count: screenshotsRep.count
    property int currentIndex: -1
    property Item currentItem: screenshotsRep.itemAt(currentIndex)

    Layout.preferredHeight: Kirigami.Units.gridUnit * 13
    contentHeight: height
    contentWidth: screenshotsLayout.width

    QQC2.Popup {
        id: overlay
        parent: applicationWindow().QQC2.Overlay.overlay
        modal: true
        clip: false

        x: (parent.width - width) / 2
        y: (parent.height - height) / 2

        readonly property real proportion: overlayImage.sourceSize.width > 1
            ? overlayImage.sourceSize.height / overlayImage.sourceSize.width
            : 1

        height: overlayImage.status === Image.Loading
            ? Kirigami.Units.gridUnit * 5
            : Math.min(parent.height * 0.9, (parent.width * 0.9) * proportion, overlayImage.sourceSize.height)
        width: height / proportion

        QQC2.BusyIndicator {
            id: indicator
            visible: running
            running: overlayImage.status === Image.Loading
            anchors.fill: parent
        }

        // Only animate the images in the detail view; in the overview it would be irritating and not useful anyway due to the small previews
        AnimatedImage {
            id: overlayImage
            anchors.fill: parent
            source: root.currentItem ? root.currentItem.imageSource : ""
            fillMode: Image.PreserveAspectFit
            smooth: true
        }

        QQC2.Button {
            anchors {
                right: parent.left
                verticalCenter: parent.verticalCenter
            }
            action: leftAction
            visible: leftAction.visible
        }

        QQC2.Button {
            anchors {
                left: parent.right
                verticalCenter: parent.verticalCenter
            }
            action: rightAction
            visible: rightAction.visible
        }

        Kirigami.Action {
            id: leftAction
            icon.name: "arrow-left"
            enabled: overlay.visible && visible
            visible: root.currentIndex >= 1 && !indicator.running
            onTriggered: source => {
                root.currentIndex = (root.currentIndex - 1) % root.count;
            }
        }

        Kirigami.Action {
            id: rightAction
            icon.name: "arrow-right"
            enabled: overlay.visible && visible
            visible: root.currentIndex < (root.count - 1) && !indicator.running
            onTriggered: source => {
                root.currentIndex = (root.currentIndex + 1) % root.count;
            }
        }
    }

    Row {
        id: screenshotsLayout
        height: root.contentHeight
        spacing: Kirigami.Units.largeSpacing
        leftPadding: spacing
        rightPadding: spacing
        focus: overlay.visible

        Keys.onLeftPressed: event => {
            if (leftAction.visible) {
                leftAction.trigger();
            }
        }
        Keys.onRightPressed: event => {
            if (rightAction.visible) {
                rightAction.trigger();
            }
        }

        Repeater {
            id: screenshotsRep

            delegate: MouseArea {
                readonly property url imageSource: modelData
                readonly property real proportion: thumbnail.sourceSize.width > 1
                    ? thumbnail.sourceSize.height / thumbnail.sourceSize.width
                    : 1

                anchors.verticalCenter: parent.verticalCenter
                width: Math.max(50, height / proportion)
                height: screenshotsLayout.height - 2 * Kirigami.Units.largeSpacing

                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor

                onClicked: mouse => {
                    root.currentIndex = index
                    overlay.open()
                }

                Kirigami.ShadowedRectangle {
                    visible: thumbnail.status === Image.Ready
                    anchors.fill: thumbnail
                    Kirigami.Theme.colorSet: Kirigami.Theme.View
                    shadow.size: Kirigami.Units.largeSpacing
                    shadow.color: Qt.rgba(0, 0, 0, 0.3)
                }

                QQC2.BusyIndicator {
                    running: thumbnail.status === Image.Loading
                    anchors.centerIn: parent
                }

                AnimatedImage {
                    id: thumbnail
                    source: modelData
                    height: parent.height
                    fillMode: Image.PreserveAspectFit
                    smooth: true
                }
            }
        }
    }
    clip: true
    readonly property var leftShadow: Shadow {
        parent: root
        anchors {
            left: parent.left
            top: parent.top
            bottom: parent.bottom
        }
        edge: Qt.LeftEdge
        width: Math.max(0, Math.min(root.width / 5, root.contentX))
    }

    readonly property var rightShadow: Shadow {
        parent: root
        anchors {
            right: parent.right
            top: parent.top
            bottom: parent.bottom
        }
        edge: Qt.RightEdge
        width: Math.max(0, Math.min(root.contentWidth - root.contentX - root.width) / 5)
    }
}
