/*
    SPDX-FileCopyrightText: 2017 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2017 The Qt Company Ltd.
    SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-or-later
*/

import QtQuick
import org.kde.qqc2desktopstyle.private as StylePrivate
import QtQuick.Templates as T
import org.kde.kirigami as Kirigami

T.ProgressBar {
    id: controlRoot

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding,
                            250)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding)

    baselineOffset: background ? background.y + background.baselineOffset : 0

    topInset: Kirigami.Units.largeSpacing
    bottomInset: Kirigami.Units.largeSpacing

    hoverEnabled: false

    contentItem: Item {}

    background: StylePrivate.StyleItem {
        elementType: "progressbar"
        control: controlRoot
        minimum: 0
        maximum: controlRoot.indeterminate ? 0 : 100000
        value: controlRoot.indeterminate ? 0 : 100000 * controlRoot.position
        horizontal: true
        enabled: controlRoot.enabled
    }
}
