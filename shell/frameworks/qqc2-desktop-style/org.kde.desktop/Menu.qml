/*
    SPDX-FileCopyrightText: 2017 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2017 The Qt Company Ltd.
    SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-or-later
*/


import QtQuick
import QtQuick.Templates as T
import org.kde.lingmoui as LingmoUI
import org.kde.qqc2desktopstyle.private as StylePrivate

T.Menu {
    id: control

    z: LingmoUI.OverlayZStacking.z

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            contentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             contentHeight + topPadding + bottomPadding)

    margins: 0

    rightPadding: (scrollbar.visible &&  !mirrored) ? scrollbar.width : undefined
    leftPadding: (scrollbar.visible &&  mirrored) ? scrollbar.width : undefined
    horizontalPadding: style.pixelMetric("menuhmargin")
    verticalPadding: style.pixelMetric("menuvmargin")

    property Item __style: StylePrivate.StyleItem {
        id: style
    }

    delegate: MenuItem {}

    contentItem: ListView {
        id: listview
        property bool hasCheckables: false
        property bool hasIcons: false

        implicitWidth: contentItem.children
            .reduce((maxWidth, child) => Math.max(maxWidth, child.implicitWidth), 0)
        // Some non-zero value, so the whole menu does not get stuck zero
        // sized. Otherwise ListView just refuses to update implicitHeight -- just zero.
        // `contentHeight` reports a wrong estimated height when all items are invisible.
        // Use childrenRect instead.
        // https://invent.kde.org/qt/qt/qtdeclarative/-/blob/8a0787f3bbed226785c842e1fd273a5b6dc06a32/src/quick/items/qquicklistview.cpp#L520
        implicitHeight: Math.max(1, contentItem.childrenRect.height)
        model: control.contentModel

        spacing: 0 // Hardcoded to the Ocean theme value

        interactive: Window.window
                        ? contentHeight + control.topPadding + control.bottomPadding > Window.window.height
                        : false
        clip: true
        currentIndex: control.currentIndex

        keyNavigationEnabled: true
        keyNavigationWraps: true

        ScrollBar.vertical: ScrollBar {
            id: scrollbar
            parent: listview.parent
            anchors.top: listview.top
            anchors.left: listview.right
            anchors.bottom: listview.bottom
        }

        // mimic qtwidgets behaviour regarding menu highlighting
        Connections {
            target: control.contentItem.currentItem

            function onHoveredChanged() {
                const item = control.contentItem.currentItem;
                if (item instanceof T.MenuItem && item.highlighted
                        && !item.subMenu && !item.hovered) {
                    control.currentIndex = -1
                }
            }
        }
    }

    Connections {
        target: control.contentItem.contentItem

        function onVisibleChildrenChanged() {
            const children = control.contentItem.contentItem.visibleChildren;
            let hasCheckables = control.contentItem.hasCheckables;
            let hasIcons = control.contentItem.hasIcons;
            for (const child of children) {
                if (child.checkable) {
                    hasCheckables = true;
                }
                if (child.icon && child.icon.hasOwnProperty("name")
                        && (child.icon.name !== "" || child.icon.source.toString() !== "")) {
                    hasIcons = true;
                }
            }
            control.contentItem.hasCheckables = hasCheckables;
            control.contentItem.hasIcons = hasIcons;
        }
    }

    enter: Transition {
        NumberAnimation {
            property: "opacity"
            from: 0
            to: 1
            easing.type: Easing.InOutQuad
            duration: LingmoUI.Units.shortDuration
        }
    }

    exit: Transition {
        NumberAnimation {
            property: "opacity"
            from: 1
            to: 0
            easing.type: Easing.InOutQuad
            duration: LingmoUI.Units.shortDuration
        }
    }

    background: LingmoUI.ShadowedRectangle {
        radius: LingmoUI.Units.cornerRadius
        implicitWidth: LingmoUI.Units.gridUnit * 8
        color: LingmoUI.Theme.backgroundColor

        border.color: LingmoUI.ColorUtils.linearInterpolation(LingmoUI.Theme.backgroundColor, LingmoUI.Theme.textColor, LingmoUI.Theme.frameContrast)
        border.width: 1

        shadow.xOffset: 0
        shadow.yOffset: 2
        shadow.color: Qt.rgba(0, 0, 0, 0.3)
        shadow.size: 8
    }
}
