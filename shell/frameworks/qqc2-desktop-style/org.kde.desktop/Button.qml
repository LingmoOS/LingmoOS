/*
    SPDX-FileCopyrightText: 2017 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2017 The Qt Company Ltd.
    SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Templates as T
import org.kde.lingmoui as LingmoUI
import org.kde.qqc2desktopstyle.private as StylePrivate

T.Button {
    id: controlRoot

    LingmoUI.Theme.colorSet: LingmoUI.Theme.Button
    LingmoUI.Theme.inherit: false

    implicitWidth: Math.max((text && display !== T.AbstractButton.IconOnly ?
        implicitBackgroundWidth : implicitHeight) + leftInset + rightInset,
        implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding)

    baselineOffset: background ? background.y + background.baselineOffset : 0

    hoverEnabled: Qt.styleHints.useHoverEffects

    LingmoUI.MnemonicData.enabled: enabled && visible
    LingmoUI.MnemonicData.controlType: LingmoUI.MnemonicData.ActionElement
    LingmoUI.MnemonicData.label: display !== T.AbstractButton.IconOnly ? text : ""
    Shortcut {
        //in case of explicit & the button manages it by itself
        enabled: !(RegExp(/\&[^\&]/).test(controlRoot.text))
        sequence: controlRoot.LingmoUI.MnemonicData.sequence
        onActivated: controlRoot.clicked()
    }
    background: StylePrivate.StyleItem {
        control: controlRoot
        elementType: "button"
        sunken: controlRoot.down
        on: controlRoot.checkable && controlRoot.checked
        flat: controlRoot.flat
        hover: controlRoot.hovered
        text: controlRoot.LingmoUI.MnemonicData.mnemonicLabel
        hasFocus: controlRoot.activeFocus || controlRoot.highlighted
        activeControl: controlRoot.Accessible.defaultButton ? "default" : ""
        properties: {
            "icon": controlRoot.display !== T.AbstractButton.TextOnly
                ? (controlRoot.icon.name !== "" ? controlRoot.icon.name : controlRoot.icon.source) : null,
            "iconColor": Qt.colorEqual(controlRoot.icon.color, "transparent") ? LingmoUI.Theme.textColor : controlRoot.icon.color,
            "iconWidth": controlRoot.icon.width,
            "iconHeight": controlRoot.icon.height,

            "menu": controlRoot.Accessible.role === Accessible.ButtonMenu
        }
    }
}
