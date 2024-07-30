/*
 *  SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import QtQuick.Templates as T

import org.kde.kirigami 2.20 as Kirigami
import org.kde.ksvg 1.0 as KSvg

T.ToolBar {
    id: control

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            contentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             contentHeight + topPadding + bottomPadding)

    topPadding: background.margins.top
    leftPadding: background.margins.left
    rightPadding: background.margins.right
    bottomPadding: background.margins.bottom

    spacing: Kirigami.Units.smallSpacing

    background: KSvg.FrameSvgItem {
        imagePath: "widgets/background"
    }
}
