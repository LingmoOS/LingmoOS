/*
    SPDX-FileCopyrightText: 2017 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2017 The Qt Company Ltd.

    SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-or-later
*/


import QtQuick
import QtQuick.Templates as T
import org.kde.kirigami as Kirigami

T.Drawer {
    id: control

    z: Kirigami.OverlayZStacking.z

    parent: T.ApplicationWindow.overlay

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            contentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             contentHeight + topPadding + bottomPadding)

    topPadding: edge === Qt.BottomEdge ? 1 : 0
    leftPadding: edge === Qt.RightEdge ? 1 : 0
    rightPadding: edge === Qt.LeftEdge ? 1 : 0
    bottomPadding: edge === Qt.TopEdge ? 1 : 0

    background: Rectangle {
        color: Kirigami.Theme.backgroundColor
        Kirigami.Separator {
            readonly property bool horizontal: control.edge === Qt.LeftEdge || control.edge === Qt.RightEdge

            width: horizontal ? 1 : parent.width
            height: horizontal ? parent.height : 1
            x: control.edge === Qt.LeftEdge ? parent.width - 1 : 0
            y: control.edge === Qt.TopEdge ? parent.height - 1 : 0

            Kirigami.Theme.inherit: false
            Kirigami.Theme.colorSet: Kirigami.Theme.Header
        }
    }

    enter: Transition {
        SmoothedAnimation {
            velocity: 5
        }
    }
    exit: Transition {
        SmoothedAnimation {
            velocity: 5
        }
    }
}
