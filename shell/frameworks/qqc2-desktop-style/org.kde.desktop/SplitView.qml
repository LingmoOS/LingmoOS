/*
    SPDX-FileCopyrightText: 2018 The Qt Company Ltd.
    SPDX-FileCopyrightText: 2024 ivan tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-or-later
*/

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Templates as T
import org.kde.kirigami.platform as Platform
import org.kde.qqc2desktopstyle.private as StylePrivate

T.SplitView {
    id: control

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding)

    handle: StylePrivate.StyleItem {
        id: handle

        elementType: "splitter"
        horizontal: control.orientation === Qt.Horizontal

        // Increase the hit area
        //
        // It could be adapted from QSplitterHandle::resizeEvent, but its
        // extra margins are kinda too small. Or it could be hardcoded to
        // Breeze -> SplitterProxyWidth config which is 12, that is, assuming
        // the feature is enabled at all.

        // Increase the hit area
        containmentMask: Item {
            // Dynamic margins like in Breeze/SplitterProxy
            readonly property int handleMargin: handle.T.SplitHandle.hovered ? 12 : 6

            x: handle.horizontal ? -handleMargin : 0
            y: handle.horizontal ? 0 : -handleMargin

            width: handle.width + (!handle.horizontal ? 0 : handleMargin * 2)
            height: handle.height + (handle.horizontal ? 0 : handleMargin * 2)
        }
    }
}
