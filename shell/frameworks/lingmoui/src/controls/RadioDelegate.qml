/*
 * Copyright (C) 2024 Lingmo OS Team <team@lingmo.org>
 *
 * Author: Lingmo OS Team <team@lingmo.org>
 *
 * GNU Lesser General Public License Usage
 * Alternatively, this file may be used under the terms of the GNU Lesser
 * General Public License version 3 as published by the Free Software
 * Foundation and appearing in the file LICENSE.LGPLv3 included in the
 * packaging of this file. Please review the following information to
 * ensure the GNU Lesser General Public License version 3 requirements
 * will be met: https://www.gnu.org/licenses/lgpl.html.
 *
 * GNU General Public License Usage
 * Alternatively, this file may be used under the terms of the GNU
 * General Public License version 2.0 or later as published by the Free
 * Software Foundation and appearing in the file LICENSE.GPL included in
 * the packaging of this file. Please review the following information to
 * ensure the GNU General Public License version 2.0 requirements will be
 * met: http://www.gnu.org/licenses/gpl-2.0.html.
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Templates as T
import org.kde.lingmoui as LingmoUI

T.RadioDelegate {
    id: control

    implicitWidth: Math.max(background ? background.implicitWidth : 0,
                            contentItem.implicitWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(background ? background.implicitHeight : 0,
                             Math.max(contentItem.implicitHeight,
                                      indicator ? indicator.implicitHeight : 0) + topPadding + bottomPadding)
    baselineOffset: contentItem.y + contentItem.baselineOffset

    topPadding: control.LingmoUI.Units.smallSpacing
    bottomPadding: control.LingmoUI.Units.smallSpacing
    leftPadding: control.LingmoUI.Units.smallSpacing
    rightPadding: control.LingmoUI.Units.smallSpacing

    spacing: control.LingmoUI.Units.smallSpacing

    opacity: control.enabled ? 1.0 : 0.5

    contentItem: Text {
        leftPadding: !control.mirrored ? 0 : control.indicator.width + control.spacing
        rightPadding: control.mirrored ? 0 : control.indicator.width + control.spacing

        text: control.text
        font: control.font
        color: LingmoUI.Theme.textColor

        elide: Text.ElideRight
        visible: control.text
        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignVCenter
    }

    indicator: RadioIndicator {
        x: control.mirrored ? control.leftPadding : control.width - width - control.rightPadding
        y: control.topPadding + (control.availableHeight - height) / 2

        control: control
    }

    background: Rectangle {
        implicitHeight: control.Suru.units.gu(7)
        color: control.Suru.backgroundColor

        HighlightFocusRectangle {
            control: control
            width: parent.width
            height: parent.height
        }

        Rectangle {
            anchors.bottom: parent.bottom
            width: parent.width
            height: control.Suru.units.dp(1)
            color: control.Suru.neutralColor
        }
    }
}