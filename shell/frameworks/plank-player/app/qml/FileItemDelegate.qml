// SPDX-FileCopyrightText: 2021 Aditya Mehra <aix.m@outlook.com>
//
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import org.kde.lingmoui 2.19 as LingmoUI
import Qt5Compat.GraphicalEffects

ItemDelegate {
    id: delegate
    property int borderSize: LingmoUI.Units.smallSpacing
    property int baseRadius: 3

    readonly property Flickable gridView: {
        var candidate = parent;
        while (candidate) {
            if (candidate instanceof Flickable) {
                return candidate;
            }
            candidate = candidate.parent;
        }
        return null;
    }

    readonly property bool isCurrent: {
        gridView.currentIndex == index && activeFocus && !gridView.moving
    }

    leftPadding: LingmoUI.Units.largeSpacing * 2
    topPadding: LingmoUI.Units.largeSpacing * 2
    rightPadding: LingmoUI.Units.largeSpacing * 2
    bottomPadding: LingmoUI.Units.largeSpacing * 2

    leftInset: LingmoUI.Units.largeSpacing
    topInset: LingmoUI.Units.largeSpacing
    rightInset: LingmoUI.Units.largeSpacing
    bottomInset: LingmoUI.Units.largeSpacing

    implicitWidth: gridView.cellWidth
    height: gridView.cellHeight

    background: Item {
        id: background

        readonly property Item highlight: Rectangle {
            parent: delegate
            z: 1
            anchors {
                fill: parent
            }
            color: "transparent"
            border {
                width: delegate.borderSize
                color: delegate.LingmoUI.Theme.highlightColor
            }
            opacity: delegate.isCurrent || delegate.highlighted
            Behavior on opacity {
                OpacityAnimator {
                    duration: LingmoUI.Units.longDuration/2
                    easing.type: Easing.InOutQuad
                }
            }
        }

        Rectangle {
            id: frame
            anchors {
                fill: parent
            }
            radius: delegate.baseRadius
            color: delegate.LingmoUI.Theme.backgroundColor
            layer.enabled: true
            layer.effect: DropShadow {
                transparentBorder: false
                horizontalOffset: 1.25
                verticalOffset: 1
            }

            states: [
                State {
                    when: delegate.isCurrent
                    PropertyChanges {
                        target: delegate
                        leftInset: 0
                        rightInset: 0
                        topInset: 0
                        bottomInset: 0
                    }
                    PropertyChanges {
                        target: background.highlight.anchors
                        margins: 0
                    }
                    PropertyChanges {
                        target: frame
                        // baseRadius + borderSize preserves the original radius for the visible part of frame
                        radius: delegate.baseRadius + delegate.borderSize
                    }
                    PropertyChanges {
                        target: background.highlight
                        // baseRadius + borderSize preserves the original radius for the visible part of frame
                        radius: delegate.baseRadius + delegate.borderSize
                    }
                },
                State {
                    when: !delegate.isCurrent
                    PropertyChanges {
                        target: delegate
                        leftInset: LingmoUI.Units.largeSpacing
                        rightInset: LingmoUI.Units.largeSpacing
                        topInset: LingmoUI.Units.largeSpacing
                        bottomInset: LingmoUI.Units.largeSpacing
                    }
                    PropertyChanges {
                        target: background.highlight.anchors
                        margins: LingmoUI.Units.largeSpacing
                    }
                    PropertyChanges {
                        target: frame
                        radius: delegate.baseRadius
                    }
                    PropertyChanges {
                        target: background.highlight
                        radius: delegate.baseRadius
                    }
                }
            ]

            transitions: Transition {
                ParallelAnimation {
                    NumberAnimation {
                        property: "leftInset"
                        duration: LingmoUI.Units.longDuration
                        easing.type: Easing.InOutQuad
                    }
                    NumberAnimation {
                        property: "rightInset"
                        duration: LingmoUI.Units.longDuration
                        easing.type: Easing.InOutQuad
                    }
                    NumberAnimation {
                        property: "topInset"
                        duration: LingmoUI.Units.longDuration
                        easing.type: Easing.InOutQuad
                    }
                    NumberAnimation {
                        property: "bottomInset"
                        duration: LingmoUI.Units.longDuration
                        easing.type: Easing.InOutQuad
                    }
                    NumberAnimation {
                        property: "radius"
                        duration: LingmoUI.Units.longDuration
                        easing.type: Easing.InOutQuad
                    }
                    NumberAnimation {
                        property: "margins"
                        duration: LingmoUI.Units.longDuration
                        easing.type: Easing.InOutQuad
                    }
                }
            }
        }
    }

    contentItem: ColumnLayout {
        spacing: LingmoUI.Units.smallSpacing

        Item {
            id: imgRoot
            //clip: true
            Layout.alignment: Qt.AlignTop
            Layout.fillWidth: true
            Layout.topMargin: -delegate.topPadding + delegate.topInset + extraBorder
            Layout.leftMargin: -delegate.leftPadding + delegate.leftInset + extraBorder
            Layout.rightMargin: -delegate.rightPadding + delegate.rightInset + extraBorder
            // Any width times 0.5625 is a 16:9 ratio
            // Adding baseRadius is needed to prevent the bottom from being rounded
            Layout.preferredHeight: width * 0.5625 + delegate.baseRadius
            // FIXME: another thing copied from AbstractDelegate
            property real extraBorder: 0

            layer.enabled: true
            layer.effect: OpacityMask {
                cached: true
                maskSource: Rectangle {
                    x: imgRoot.x;
                    y: imgRoot.y
                    width: imgRoot.width
                    height: imgRoot.height
                    radius: delegate.baseRadius
                }
            }

            LingmoUI.Icon {
                id: img
                source: "kdenlive-show-video"
                anchors {
                    fill: parent
                    // To not round under
                    bottomMargin: delegate.baseRadius
                }
                opacity: 1
            }

            states: [
                State {
                    when: delegate.isCurrent
                    PropertyChanges {
                        target: imgRoot
                        extraBorder: delegate.borderSize
                    }
                },
                State {
                    when: !delegate.isCurrent
                    PropertyChanges {
                        target: imgRoot
                        extraBorder: 0
                    }
                }
            ]
            transitions: Transition {
                onRunningChanged: {
                    // Optimize when animating the thumbnail
                    img.smooth = !running
                }
                NumberAnimation {
                    property: "extraBorder"
                    duration: LingmoUI.Units.longDuration
                    easing.type: Easing.InOutQuad
                }
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            // Compensate for blank space created from not rounding thumbnail bottom corners
            Layout.topMargin: -delegate.baseRadius
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            spacing: LingmoUI.Units.smallSpacing

            LingmoUI.Heading {
                id: videoLabel
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                wrapMode: Text.Wrap
                level: 3
                maximumLineCount: 1
                elide: Text.ElideRight
                color: LingmoUI.Theme.textColor
                Component.onCompleted: {
                    text = fileName + (fileIsDir ? "/" : "")
                }
            }
        }
    }

    Keys.onReturnPressed: (event)=> {
        clicked()
    }
}
