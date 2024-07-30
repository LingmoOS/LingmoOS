/*
    SPDX-FileCopyrightText: 2017 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2017 The Qt Company Ltd.
    SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-or-later
*/


import QtQuick
import QtQuick.Templates as T
import org.kde.kirigami as Kirigami
import org.kde.qqc2desktopstyle.private as StylePrivate

T.ScrollView {
    id: controlRoot

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            contentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             contentHeight + topPadding + bottomPadding)

    Kirigami.Theme.colorSet: Kirigami.Theme.View
    Kirigami.Theme.inherit: !background || !background.visible

    // size in pixel to accommodate the border drawn by qstyle
    topPadding: controlRoot.background?.visible ? (background.topPadding ?? 0) : 0
    leftPadding: (controlRoot.background?.visible ? (background.leftPadding ?? 0) : 0)
                    + (mirrored ? internal.verticalScrollBarWidth : 0)
    rightPadding: (controlRoot.background?.visible ? (background.rightPadding ?? 0) : 0)
                    + (!mirrored ? internal.verticalScrollBarWidth : 0)
    bottomPadding: (controlRoot.background?.visible ? (background.bottomPadding ?? 0) : 0)
                    + internal.horizontalScrollBarHeight

    //create a background only after Component.onCompleted, see on the component creation below for explanation
    Component.onCompleted: {
        if (!controlRoot.background) {
            controlRoot.background = backgroundComponent.createObject(controlRoot);
        }
    }

    data: [
        Kirigami.WheelHandler {
            target: controlRoot.contentItem
        },
        // create a background only after Component.onCompleted because:
        // implementations can set their own background in a declarative way.
        // ScrollView {background.visible: true} must *not* work, because all
        // upstream styles don't have a background so applications using this
        // would break with other styles.
        Component {
            id: backgroundComponent
            StylePrivate.StyleItem {
                id: styled
                control: controlRoot
                elementType: "frame"
                visible: false
                sunken: true
                raised: false
                enabled: controlRoot.contentItem.enabled
                hasFocus: controlRoot.activeFocus || controlRoot.contentItem.activeFocus
                hover: controlRoot.hovered
            }
        },

        QtObject {
            id: internal

            readonly property real verticalScrollBarWidth: controlRoot.ScrollBar.vertical.visible && controlRoot.ScrollBar.vertical.interactive ? controlRoot.ScrollBar.vertical.width : 0
            readonly property real horizontalScrollBarHeight: controlRoot.ScrollBar.horizontal.visible && controlRoot.ScrollBar.vertical.interactive ? controlRoot.ScrollBar.horizontal.height : 0
        }
    ]

    ScrollBar.vertical: ScrollBar {
        parent: controlRoot
        z: 1
        x: controlRoot.mirrored
            ? (controlRoot.background?.visible ? (controlRoot.background.leftPadding ?? 0) : 0)
            : controlRoot.width - width - (controlRoot.background?.visible ? (controlRoot.background.rightPadding ?? 0) : 0)
        y: controlRoot.topPadding
        height: controlRoot.availableHeight
        active: controlRoot.ScrollBar.horizontal.active
    }

    ScrollBar.horizontal: ScrollBar {
        parent: controlRoot
        z: 1
        x: controlRoot.leftPadding
        y: controlRoot.height - height - (controlRoot.background?.visible ? (controlRoot.background.bottomPadding ?? 0) : 0)
        width: controlRoot.availableWidth
        active: controlRoot.ScrollBar.vertical.active
    }
}
