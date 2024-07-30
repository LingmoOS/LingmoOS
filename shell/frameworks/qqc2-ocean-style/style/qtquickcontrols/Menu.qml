/*
    SPDX-FileCopyrightText: 2017 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2017 The Qt Company Ltd.
    SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
    SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-or-later
*/


import QtQuick
import QtQuick.Layouts
import QtQuick.Window
import QtQuick.Controls as Controls
import QtQuick.Controls.impl
import QtQuick.Templates as T
import org.kde.lingmoui as LingmoUI

import org.kde.ocean.impl as Impl

T.Menu {
    id: control

    readonly property bool __hasIndicators: contentItem.contentItem.visibleChildren.some(menuItem => menuItem?.indicator?.visible ?? false)
    readonly property bool __hasIcons: contentItem.contentItem.visibleChildren.some(menuItem => __itemHasIcon(menuItem))
    readonly property bool __hasArrows: contentItem.contentItem.visibleChildren.some(menuItem => menuItem?.arrow?.visible ?? false)

    // palette: LingmoUI.Theme.palette
    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            contentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             contentHeight + topPadding + bottomPadding)
    padding: LingmoUI.Units.smallSpacing
    margins: 0
    overlap: background && background.hasOwnProperty("border") ? background.border.width : 0

    function __itemHasIcon(item) {
        const hasName = (item?.icon?.name ?? "") !== ""
        const hasSource = (item?.icon?.source.toString() ?? "") !== ""
        return hasName || hasSource
    }

    // The default contentItem is a ListView, which has its own contentItem property,
    // so delegates will be created as children of control.contentItem.contentItem
    delegate: Controls.MenuItem {}

    contentItem: ListView {
        implicitHeight: contentHeight
        // Cannot use `contentWidth` as this only accounts for Actions, not MenuItems or MenuSeparators
        implicitWidth: contentItem.visibleChildren.reduce((maxWidth, child) => Math.max(maxWidth, child.implicitWidth), 0)
        model: control.contentModel
        highlightMoveDuration: LingmoUI.Units.shortDuration
        highlightMoveVelocity: 800
        highlight: Impl.ListViewHighlight {
            currentIndex: control.currentIndex
            count: control.count
            alwaysCurveCorners: true
        }
        // For some reason, `keyNavigationEnabled: true` isn't needed and
        // using it causes separators and disabled items to be highlighted
        keyNavigationWraps: true

        // Makes it so you can't drag/flick the list view around unless the menu is taller than the window
        interactive: Window.window ? contentHeight + control.topPadding + control.bottomPadding > Window.window.height : false
        clip: interactive // Only needed when the ListView can be dragged/flicked
        currentIndex: control.currentIndex || 0

        ScrollBar.vertical: Controls.ScrollBar {}
    }

    enter: Transition {
        ParallelAnimation {
            NumberAnimation {
                property: "opacity"
                from: 0
                to: 1
                easing.type: Easing.OutCubic
                duration: LingmoUI.Units.shortDuration
            }
        }
    }

    exit: Transition {
        ParallelAnimation {
            NumberAnimation {
                property: "opacity"
                from: 1
                to: 0
                easing.type: Easing.InCubic
                duration: LingmoUI.Units.shortDuration
            }
        }
    }

    background: Rectangle {
        radius: Impl.Units.smallRadius
        implicitHeight: Impl.Units.mediumControlHeight
        implicitWidth: LingmoUI.Units.gridUnit * 8
        color: LingmoUI.Theme.backgroundColor

        border {
            color: Impl.Theme.separatorColor()
            width: Impl.Units.smallBorder
        }

        Impl.LargeShadow {
            radius: parent.radius
        }
    }
}
