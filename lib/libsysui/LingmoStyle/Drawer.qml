/*
 * SPDX-FileCopyrightText: 2024 Elysia <elysia@lingmo.org>
 *
 * SPDX-License-Identifier: GPL-3.0
 */

import QtQuick
import QtQuick.Templates as T
import LingmoUI

T.Drawer {
    id: control

    parent: T.Overlay.overlay

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            contentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             contentHeight + topPadding + bottomPadding)

    topPadding: control.edge === Qt.BottomEdge
    leftPadding: control.edge === Qt.RightEdge
    rightPadding: control.edge === Qt.LeftEdge
    bottomPadding: control.edge === Qt.TopEdge

    enter: Transition { SmoothedAnimation { velocity: 5 } }
    exit: Transition { SmoothedAnimation { velocity: 5 } }

    background: Rectangle {
        color: control.Universal.chromeMediumLowColor
        Rectangle {
            readonly property bool horizontal: control.edge === Qt.LeftEdge || control.edge === Qt.RightEdge
            width: horizontal ? 1 : parent.width
            height: horizontal ? parent.height : 1
            radius: LingmoUnits.windowRadius
            color: LingmoTheme.dark ? Qt.rgba(32/255,32/255,32/255,1) : Qt.rgba(243/255,243/255,243/255,1)
            x: control.edge === Qt.LeftEdge ? parent.width - 1 : 0
            y: control.edge === Qt.TopEdge ? parent.height - 1 : 0
        }
    }

    T.Overlay.modal: Rectangle {
        color: LingmoTheme.dark ? Qt.rgba(32/255,32/255,32/255,1) : Qt.rgba(243/255,243/255,243/255,1)
    }

    T.Overlay.modeless: Rectangle {
        color: LingmoTheme.dark ? Qt.rgba(32/255,32/255,32/255,1) : Qt.rgba(243/255,243/255,243/255,1)
    }
}
