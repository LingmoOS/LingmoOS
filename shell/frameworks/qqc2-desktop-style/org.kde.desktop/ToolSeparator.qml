/*
    SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>

    SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-or-later
*/


import QtQuick
import QtQuick.Templates as T
import org.kde.kirigami as Kirigami

T.ToolSeparator {
    id: controlRoot

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding)

    contentItem: Kirigami.Separator {
        // implicitHeight is the same as ToolBar implicitHeight minus ToolBar padding if not horizontal
        implicitWidth: !controlRoot.horizontal ? 1 : 40 - (Kirigami.Units.smallSpacing * 2)
        implicitHeight: controlRoot.horizontal ? 1 : 40 - (Kirigami.Units.smallSpacing * 2)
    }
}
