// SPDX-FileCopyrightText: 2016 Marco Martin <mart@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import QtQuick.Templates as T
import org.kde.lingmoui as LingmoUI

T.SwipeView {
    id: control
    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding)


    contentItem: ListView {
        model: control.contentModel
        interactive: control.interactive
        currentIndex: control.currentIndex
        focus: control.focus

        spacing: control.spacing
        orientation: control.orientation
        snapMode: ListView.SnapOneItem
        boundsBehavior: Flickable.StopAtBounds

        highlightRangeMode: ListView.StrictlyEnforceRange
        preferredHighlightBegin: 0
        preferredHighlightEnd: 0
        highlightMoveDuration: LingmoUI.Units.longDuration
        maximumFlickVelocity: 4 * (control.orientation === Qt.Horizontal ? width : height)
    }
}
