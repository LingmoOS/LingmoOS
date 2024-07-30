/* SPDX-FileCopyrightText: 2021 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

import QtQuick
import QtQuick.Templates as T
import QtQuick.Controls as QQC2

import org.kde.lingmoui as LingmoUI

T.ScrollView {
    id: control
    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            contentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             contentHeight + topPadding + bottomPadding)

    leftPadding: mirrored && T.ScrollBar.vertical && T.ScrollBar.vertical.visible && !LingmoUI.Settings.isMobile ? T.ScrollBar.vertical.width : 0
    rightPadding: !mirrored && T.ScrollBar.vertical && T.ScrollBar.vertical.visible && !LingmoUI.Settings.isMobile ? T.ScrollBar.vertical.width : 0
    bottomPadding: T.ScrollBar.horizontal && T.ScrollBar.horizontal.visible && !LingmoUI.Settings.isMobile ? T.ScrollBar.horizontal.height : 0

    data: [
        LingmoUI.WheelHandler {
            id: wheelHandler
            target: control.contentItem
        }
    ]

    T.ScrollBar.vertical: QQC2.ScrollBar {
        parent: control
        x: control.mirrored ? 0 : control.width - width
        y: control.topPadding
        height: control.availableHeight
        active: control.T.ScrollBar.horizontal.active
        stepSize: wheelHandler.verticalStepSize / control.contentHeight
    }

    T.ScrollBar.horizontal: QQC2.ScrollBar {
        parent: control
        x: control.leftPadding
        y: control.height - height
        width: control.availableWidth
        active: control.T.ScrollBar.vertical.active
        stepSize: wheelHandler.horizontalStepSize / control.contentWidth
    }
}
