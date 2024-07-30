/*
    SPDX-FileCopyrightText: 2019 Aditya Mehra <aix.m@outlook.com>
    SPDX-FileCopyrightText: 2019 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.14
import QtQuick.Layouts 1.14
import Qt5Compat.GraphicalEffects

import org.kde.lingmo.core 2.0 as LingmoCore
import org.kde.lingmo.components 3.0 as LingmoComponents
import org.kde.lingmoui 2.12 as LingmoUI
import org.kde.mycroft.bigscreen 1.0 as BigScreen

LingmoComponents.ItemDelegate {
    id: delegate

    readonly property Flickable listView: {
        var candidate = parent;
        while (candidate) {
            if (candidate instanceof Flickable) {
                return candidate;
            }
            candidate = candidate.parent;
        }
        return null;
    }
    readonly property bool isCurrent: {//print(text+index+" "+listView.currentIndex+activeFocus+" "+listView.moving)
        listView.currentIndex == index && activeFocus && !listView.moving
    }

    highlighted: isCurrent
    property int borderSize: LingmoUI.Units.smallSpacing
    property int baseRadius: 3
    
    z: isCurrent ? 2 : 0

    onClicked: {
        listView.forceActiveFocus()
        listView.currentIndex = index
    }

    leftPadding: LingmoUI.Units.largeSpacing * 2
    topPadding: LingmoUI.Units.largeSpacing * 2
    rightPadding: LingmoUI.Units.largeSpacing * 2
    bottomPadding: LingmoUI.Units.largeSpacing * 2

    leftInset: LingmoUI.Units.largeSpacing
    topInset: LingmoUI.Units.largeSpacing
    rightInset: LingmoUI.Units.largeSpacing
    bottomInset: LingmoUI.Units.largeSpacing


    Keys.onReturnPressed: {
        clicked();
    }

    contentItem: Item {}

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

        LingmoUI.ShadowedRectangle {
            id: frame
            anchors {
                fill: parent
            }
            radius: delegate.baseRadius
            color: delegate.LingmoUI.Theme.backgroundColor
            shadow {
                size: LingmoUI.Units.largeSpacing * 2
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
}
