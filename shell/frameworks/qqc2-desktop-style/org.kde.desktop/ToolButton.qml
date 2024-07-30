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

T.ToolButton {
    id: controlRoot

    Kirigami.Theme.colorSet: flat ? Kirigami.Theme.Window : Kirigami.Theme.Button
    Kirigami.Theme.inherit: flat

    implicitWidth: Math.max((text && display !== T.AbstractButton.IconOnly ?
        implicitBackgroundWidth : implicitHeight) + leftInset + rightInset,
        implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding)

    baselineOffset: background ? background.y + background.baselineOffset : 0

    hoverEnabled: Qt.styleHints.useHoverEffects

    flat: true
    Kirigami.MnemonicData.enabled: enabled && visible
    Kirigami.MnemonicData.controlType: Kirigami.MnemonicData.SecondaryControl
    Kirigami.MnemonicData.label: text

    // KF6 TODO: investigate setting this by default
    // focusPolicy: Qt.TabFocus

    Shortcut {
        //in case of explicit & the button manages it by itself
        enabled: !(RegExp(/\&[^\&]/).test(controlRoot.text))
        sequence: controlRoot.Kirigami.MnemonicData.sequence
        onActivated: controlRoot.clicked()
    }
    background: StylePrivate.StyleItem {
        id: styleitem
        control: controlRoot
        elementType: "toolbutton"
        sunken: controlRoot.down
        on: controlRoot.checkable && controlRoot.checked
        hover: controlRoot.hovered
        text: controlRoot.Kirigami.MnemonicData.mnemonicLabel
        hasFocus: controlRoot.visualFocus || (!controlRoot.flat && controlRoot.pressed) || controlRoot.highlighted
        flat: controlRoot.flat
        font: controlRoot.font

        // note: keep in sync with DelayButton
        readonly property int toolButtonStyle: {
            switch (controlRoot.display) {
            case T.AbstractButton.IconOnly: return Qt.ToolButtonIconOnly;
            case T.AbstractButton.TextOnly: return Qt.ToolButtonTextOnly;
            case T.AbstractButton.TextBesideIcon:
            case T.AbstractButton.TextUnderIcon:
                // TODO KF6: check if this condition is still needed
                if (controlRoot.icon.name !== "" || controlRoot.icon.source.toString() !== "") {
                    // has icon
                    switch (controlRoot.display) {
                        case T.AbstractButton.TextBesideIcon: return Qt.ToolButtonTextBesideIcon;
                        case T.AbstractButton.TextUnderIcon: return Qt.ToolButtonTextUnderIcon;
                    }
                } else {
                    return Qt.ToolButtonTextOnly;
                }
            default: return Qt.ToolButtonFollowStyle;
            }
        }

        properties: {
            "icon": controlRoot.icon.name !== "" ? controlRoot.icon.name : controlRoot.icon.source,
            "iconColor": Qt.colorEqual(controlRoot.icon.color, "transparent") ? Kirigami.Theme.textColor : controlRoot.icon.color,
            "iconWidth": controlRoot.icon.width,
            "iconHeight": controlRoot.icon.height,

            "menu": controlRoot.Accessible.role === Accessible.ButtonMenu,
            "toolButtonStyle": toolButtonStyle,
        }
    }
}
