/*
    SPDX-FileCopyrightText: 2017 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2017 The Qt Company Ltd.

    SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-or-later
*/


import QtQuick
import QtQuick.Templates as T
import org.kde.kirigami as Kirigami
import org.kde.desktop.private as Private

T.ToolBar {
    id: controlRoot

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            contentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             contentHeight + topPadding + bottomPadding)

    padding: Kirigami.Units.smallSpacing

    // Note: relying on this heuristic might break your apps if used with other QQC2 styles.
    position: parent?.footer === controlRoot ? T.ToolBar.Footer : T.ToolBar.Header

    Kirigami.Theme.colorSet: position === T.ToolBar.Footer ? Kirigami.Theme.Window : Kirigami.Theme.Header
    Kirigami.Theme.inherit: false

    background: Private.DefaultToolBarBackground {
        control: controlRoot
    }
}
