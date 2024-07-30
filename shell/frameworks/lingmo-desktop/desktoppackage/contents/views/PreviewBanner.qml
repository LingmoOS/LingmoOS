/*
    SPDX-FileCopyrightText: 2023 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick

import org.kde.lingmo.extras as LingmoExtras
import org.kde.lingmoui 2.20 as LingmoUI

Item {
    // Using childrenRect.width causes a binding loop since we can only get the
    // actual width, not the implicitWidth--which is what we would want
    width: Math.max(title.implicitWidth, subtitle.implicitWidth)
    height: childrenRect.height

    HoverHandler {
        cursorShape: Qt.PointingHandCursor
    }

    TapHandler {
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        onTapped: desktop.showPreviewBannerMenu(mapToGlobal(point.position))
    }

    LingmoExtras.ShadowedLabel {
        id: title
        anchors {
            top: parent.top
            right: parent.right
        }
        z: 2
        text: desktop.previewBannerTitle
        // Emulate the size of a level 1 heading
        font.pointSize: Math.round(LingmoUI.Theme.defaultFont.pointSize * 1.35)
    }

    LingmoExtras.ShadowedLabel {
        id: subtitle
        anchors {
            top: title.bottom
            right: parent.right
        }
        z: 2
        text: desktop.previewBannerText
    }
}
