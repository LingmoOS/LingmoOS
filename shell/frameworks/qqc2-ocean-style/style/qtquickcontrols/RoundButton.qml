//NOTE: change this
/*
    SPDX-FileCopyrightText: 2018 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2017 The Qt Company Ltd.

    SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Templates as T
import QtQuick.Controls as Controls
import org.kde.lingmoui as LingmoUI
import org.kde.ocean.impl as Impl

T.RoundButton {
    id: control

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding,
                             implicitIndicatorHeight + topPadding + bottomPadding)

    baselineOffset: contentItem.y + contentItem.baselineOffset

    // palette: LingmoUI.Theme.palette
    LingmoUI.Theme.colorSet: control.highlighted ? LingmoUI.Theme.Selection : LingmoUI.Theme.Button
    LingmoUI.Theme.inherit: control.flat && !control.down && !control.checked

    padding: LingmoUI.Units.mediumSpacing
    spacing: LingmoUI.Units.mediumSpacing

    icon.width: LingmoUI.Units.iconSizes.sizeForLabels
    icon.height: LingmoUI.Units.iconSizes.sizeForLabels

    LingmoUI.MnemonicData.enabled: control.enabled && control.visible
    LingmoUI.MnemonicData.controlType: LingmoUI.MnemonicData.ActionElement
    LingmoUI.MnemonicData.label: control.display !== T.Button.IconOnly ? control.text : ""
    Shortcut {
        //in case of explicit & the button manages it by itself
        enabled: !(RegExp(/\&[^\&]/).test(control.text))
        sequence: control.LingmoUI.MnemonicData.sequence
        onActivated: control.clicked()
    }

    contentItem:Impl.IconLabelContent {
        control: control
        text: control.LingmoUI.MnemonicData.richTextLabel
    }

    background: Impl.ButtonBackground {
        control: control
        radius: control.radius
    }
}
