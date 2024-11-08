/*
    SPDX-FileCopyrightText: 2016 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Templates as T
import org.kde.ksvg as KSvg
//NOTE: importing LingmoCore is necessary in order to make KSvg load the current Lingmo Theme
import org.kde.lingmo.core as LingmoCore
import org.kde.lingmoui as LingmoUI

T.ToolBar {
    id: control

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            contentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             contentHeight + topPadding + bottomPadding)

    topPadding: (background as KSvg.FrameSvgItem)?.margins.top ?? undefined
    leftPadding: (background as KSvg.FrameSvgItem)?.margins.left ?? undefined
    rightPadding: (background as KSvg.FrameSvgItem)?.margins.right ?? undefined
    bottomPadding: (background as KSvg.FrameSvgItem)?.margins.bottom ?? undefined

    spacing: LingmoUI.Units.smallSpacing

    background: KSvg.FrameSvgItem {
        implicitHeight: 40 // TODO: Find a good way to sync this with the size of (Button or ToolButton) + padding
        imagePath: "widgets/toolbar"
        KSvg.SvgItem {
            imagePath: "widgets/listitem"
            elementId: "separator"
            anchors {
                left: parent.left
                right: parent.right
                top: control.position == T.ToolBar.Footer || (control.parent.footer && control.parent.footer == control) ? parent.top : undefined
                bottom: control.position == T.ToolBar.Footer || (control.parent.footer && control.parent.footer == control) ? undefined : parent.bottom
            }
        }
    }
}
