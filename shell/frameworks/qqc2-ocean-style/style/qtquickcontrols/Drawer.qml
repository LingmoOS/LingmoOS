/* SPDX-FileCopyrightText: 2017 The Qt Company Ltd.
 * SPDX-FileCopyrightText: 2017 Marco Martin <mart@kde.org>
 * SPDX-FileCopyrightText: 2021 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LicenseRef-KDE-Accepted-LGPL
 */


import QtQuick
import QtQuick.Controls
import QtQuick.Templates as T
import org.kde.lingmoui as LingmoUI
import org.kde.ocean.impl as Impl

T.Drawer {
    id: control

    parent: T.Overlay.overlay

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            contentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             contentHeight + topPadding + bottomPadding)

    topPadding: control.edge === Qt.BottomEdge ? 1 : 0
    leftPadding: control.edge === Qt.RightEdge ? 1 : 0
    rightPadding: control.edge === Qt.LeftEdge ? 1 : 0
    bottomPadding: control.edge === Qt.TopEdge ? 1 : 0

    background: LingmoUI.ShadowedRectangle {
        color: LingmoUI.Theme.backgroundColor
        shadow {
            color: Qt.rgba(0,0,0,0.2)
            size: 15
        }
        LingmoUI.Separator {
            readonly property bool horizontal: control.edge === Qt.LeftEdge || control.edge === Qt.RightEdge
            width: horizontal ? Impl.Units.smallBorder : parent.width
            height: horizontal ? parent.height : Impl.Units.smallBorder
            x: control.edge === Qt.LeftEdge ? parent.width - 1 : 0
            y: control.edge === Qt.TopEdge ? parent.height - 1 : 0
            visible: !control.dim
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

    T.Overlay.modal: Impl.OverlayModalBackground {}
    T.Overlay.modeless: Impl.OverlayDimBackground {}
}
