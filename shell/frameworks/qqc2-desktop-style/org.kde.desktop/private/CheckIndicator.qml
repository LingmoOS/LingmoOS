/*
    SPDX-FileCopyrightText: 2017 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2017 The Qt Company Ltd.
    SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-or-later
*/


import QtQuick
import QtQuick.Templates as T
import org.kde.kirigami as Kirigami
import org.kde.qqc2desktopstyle.private as StylePrivate

StylePrivate.StyleItem {
    id: styleitem

    property bool drawIcon: true
    Kirigami.Theme.inherit: false
    Kirigami.Theme.colorSet: Kirigami.Theme.Button

    readonly property T.AbstractButton buttonControl : control as T.AbstractButton

    // Fallback heuristic for MenuItem which can mimic either of those.
    elementType: (buttonControl.autoExclusive
            || (buttonControl.action !== null && buttonControl.action.T.ActionGroup.group !== null && buttonControl.action.T.ActionGroup.group.exclusive)
            || (buttonControl.T.ButtonGroup.group !== null && buttonControl.T.ButtonGroup.group.exclusive))
        ? "radiobutton" : "checkbox"

    sunken: buttonControl.pressed
    on: buttonControl.checked
    hover: buttonControl.hovered
    enabled: buttonControl.enabled
    properties: {
        "icon": drawIcon && buttonControl.display !== T.AbstractButton.TextOnly
            ? (buttonControl.icon.name !== "" ? buttonControl.icon.name : buttonControl.icon.source) : null,
        "iconColor": Qt.colorEqual(buttonControl.icon.color, "transparent") ? Kirigami.Theme.textColor : buttonControl.icon.color,
        "iconWidth": buttonControl.icon.width,
        "iconHeight": buttonControl.icon.height,

        "partiallyChecked": buttonControl.checkState === Qt.PartiallyChecked
    }
}
