/*
    SPDX-FileCopyrightText: 2016 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Templates as T
import org.kde.lingmoui as LingmoUI
import "private" as Private
//NOTE: importing LingmoCore is necessary in order to make KSvg load the current Lingmo Theme
import org.kde.lingmo.core as LingmoCore

T.Button {
    id: control

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding)

    topPadding: (background as Private.ButtonBackground)?.topMargin ?? undefined
    leftPadding: (background as Private.ButtonBackground)?.leftMargin ?? undefined
    rightPadding: (background as Private.ButtonBackground)?.rightMargin ?? undefined
    bottomPadding: (background as Private.ButtonBackground)?.bottomMargin ?? undefined

    spacing: LingmoUI.Units.smallSpacing

    hoverEnabled: !LingmoUI.Settings.tabletMode

    LingmoUI.MnemonicData.enabled: control.enabled && control.visible
    LingmoUI.MnemonicData.controlType: LingmoUI.MnemonicData.SecondaryControl
    LingmoUI.MnemonicData.label: control.text

    Shortcut {
        //in case of explicit & the button manages it by itself
        enabled: !(RegExp(/\&[^\&]/).test(control.text))
        sequence: control.LingmoUI.MnemonicData.sequence
        onActivated: control.clicked()
    }

    LingmoUI.Theme.inherit: flat
    LingmoUI.Theme.colorSet: LingmoUI.Theme.Button

    contentItem: Private.ButtonContent {
        labelText: control.LingmoUI.MnemonicData.richTextLabel
        button: control
    }

    background: Private.ButtonBackground {
        button: control
    }
}
