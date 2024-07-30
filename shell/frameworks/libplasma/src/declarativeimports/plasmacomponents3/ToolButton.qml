/*
    SPDX-FileCopyrightText: 2016 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Templates as T
import org.kde.kirigami as Kirigami
import "private" as Private

T.ToolButton {
    id: control

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding)

    topPadding: (background as Private.ButtonBackground)?.topMargin ?? undefined
    leftPadding: (background as Private.ButtonBackground)?.leftMargin ?? undefined
    rightPadding: (background as Private.ButtonBackground)?.rightMargin ?? undefined
    bottomPadding: (background as Private.ButtonBackground)?.bottomMargin ?? undefined

    spacing: Kirigami.Units.smallSpacing

    hoverEnabled: !Kirigami.Settings.tabletMode

    Accessible.onPressAction: clicked()

    Kirigami.MnemonicData.enabled: control.enabled && control.visible
    Kirigami.MnemonicData.controlType: Kirigami.MnemonicData.SecondaryControl
    Kirigami.MnemonicData.label: control.text

    // KF6 TODO: investigate setting this by default
    // focusPolicy: Qt.TabFocus

    Shortcut {
        //in case of explicit & the button manages it by itself
        enabled: !(RegExp(/\&[^\&]/).test(control.text))
        sequence: control.Kirigami.MnemonicData.sequence
        onActivated: control.clicked()
    }

    flat: true

    Kirigami.Theme.inherit: flat
    Kirigami.Theme.colorSet: Kirigami.Theme.Button

    contentItem: Private.ButtonContent {
        labelText: control.Kirigami.MnemonicData.richTextLabel
        button: control
    }

    background: Private.ButtonBackground {
        button: control
    }
}
