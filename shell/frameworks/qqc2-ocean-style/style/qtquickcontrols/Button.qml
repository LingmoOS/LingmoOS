/* SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

import QtQuick
import QtQuick.Templates as T
import org.kde.lingmoui as LingmoUI

import org.kde.ocean.impl as Impl

T.Button {
    id: control

    implicitWidth: {
        let contentAndPaddingWidth = implicitContentWidth + leftPadding + rightPadding;
        const minimumTextButtonWidth = LingmoUI.Units.iconSizes.sizeForLabels * 5;

        // To match qqc2-desktop-style behavior, we enforce a minimum width for Buttons that have text
        if (display !== AbstractButton.IconOnly && text !== "") {
            contentAndPaddingWidth = Math.max(contentAndPaddingWidth, minimumTextButtonWidth);
        }

        Math.max(implicitBackgroundWidth + leftInset + rightInset,
                 contentAndPaddingWidth,
                 implicitIndicatorWidth + leftPadding + rightPadding)
    }
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding,
                             implicitIndicatorHeight + topPadding + bottomPadding)

    flat: false

    // // palette: LingmoUI.Theme.palette

    hoverEnabled: Qt.styleHints.useHoverEffects

    LingmoUI.Theme.colorSet: /*control.highlighted ? LingmoUI.Theme.Selection :*/ LingmoUI.Theme.Button
    LingmoUI.Theme.inherit: false//control.flat && !control.down && !control.checked
    // Absolutely terrible HACK:
    // For some reason, ActionToolBar overrides the colorSet and inherit attached properties
    Component.onCompleted: {
        LingmoUI.Theme.colorSet = LingmoUI.Theme.Button/*Qt.binding(() => control.highlighted ? LingmoUI.Theme.Selection : LingmoUI.Theme.Button)*/
        LingmoUI.Theme.inherit = false//Qt.binding(() => control.flat && !(control.down || control.checked))
    }

    padding: LingmoUI.Units.largeSpacing
    leftPadding: {
        if ((!contentItem.hasIcon && contentItem.textBesideIcon) // False if contentItem has been replaced
            || display == T.AbstractButton.TextOnly
            || display == T.AbstractButton.TextUnderIcon) {
            return Impl.Units.largeHorizontalPadding;
        }
        return control.horizontalPadding;
    }
    rightPadding: {
        if (contentItem.hasLabel && display != T.AbstractButton.IconOnly) { // False if contentItem has been replaced
            return Impl.Units.largeHorizontalPadding;
        }
        return control.horizontalPadding;
    }

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

    contentItem: Impl.IconLabelContent {
        control: control
        text: control.LingmoUI.MnemonicData.richTextLabel
    }

    background: Impl.ButtonBackground {
        control: control
    }
}
