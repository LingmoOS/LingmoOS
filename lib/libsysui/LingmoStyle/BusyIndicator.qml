
/*
 * SPDX-FileCopyrightText: 2024 Elysia <elysia@lingmo.org>
 *
 * SPDX-License-Identifier: GPL-3.0
 */
import QtQuick
import QtQuick.Templates as T
import LingmoUI as LUI

T.BusyIndicator {
    id: control

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding)

    contentItem: LUI.LingmoProgressRing {
        width: control.implicitWidth
        height: control.implicitHeight
        strokeWidth: size < 40 ? 3 : 4 // "Small" vs. "Large"
        anchors.centerIn: parent
        visible: control.running
    }
}
