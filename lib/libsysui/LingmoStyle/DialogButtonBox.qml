/*
 * SPDX-FileCopyrightText: 2024 Elysia <elysia@lingmo.org>
 *
 * SPDX-License-Identifier: GPL-3.0
 */

import QtQuick
import QtQuick.Templates as T
import LingmoUI

T.DialogButtonBox {
    id: control

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            (control.count === 1 ? implicitContentWidth * 2 : implicitContentWidth) + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding)
    contentWidth: (contentItem as ListView)?.contentWidth

    spacing: 10
    padding: 24
    topPadding: position === T.DialogButtonBox.Footer ? 6 : 24
    bottomPadding: position === T.DialogButtonBox.Header ? 6 : 24
    alignment: count === 1 ? Qt.AlignRight : undefined

    delegate: LingmoButton {
        width: control.count === 1 ? control.availableWidth / 2 : undefined
    }

    contentItem: ListView {
        implicitWidth: contentWidth
        model: control.contentModel
        spacing: control.spacing
        orientation: ListView.Horizontal
        boundsBehavior: Flickable.StopAtBounds
        snapMode: ListView.SnapToItem
    }

    background: Rectangle {
        implicitHeight: 32
        radius: LingmoUnits.windowRadius
        color: LingmoTheme.dark ? Qt.rgba(32/255,32/255,32/255,1) : Qt.rgba(243/255,243/255,243/255,1)
        x: 1; y: 1
        width: parent.width - 2
        height: parent.height - 2
    }
}
