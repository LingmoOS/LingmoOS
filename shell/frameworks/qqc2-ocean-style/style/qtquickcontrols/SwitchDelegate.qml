/* SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

import QtQuick
import QtQuick.Templates as T
import QtQuick.Controls as Controls
import org.kde.lingmoui as LingmoUI
import org.kde.ocean.impl as Impl

T.SwitchDelegate {
    id: control

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding,
                            implicitIndicatorWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding,
                             implicitIndicatorHeight + topPadding + bottomPadding)

    padding: LingmoUI.Units.mediumSpacing
    leftPadding: {
        if ((!contentItem.hasIcon && contentItem.textBesideIcon) // False if contentItem has been replaced
            || display == T.AbstractButton.TextOnly
            || display == T.AbstractButton.TextUnderIcon) {
            return Impl.Units.mediumHorizontalPadding
        } else {
            return control.horizontalPadding
        }
    }
    rightPadding: {
        if (!control.indicator.visible
            && contentItem.hasLabel
            && display != T.AbstractButton.IconOnly) { // False if contentItem has been replaced
            return Impl.Units.mediumHorizontalPadding
        } else {
            return control.horizontalPadding
        }
    }

    spacing: LingmoUI.Units.mediumSpacing

    icon.width: LingmoUI.Units.iconSizes.sizeForLabels
    icon.height: LingmoUI.Units.iconSizes.sizeForLabels

    contentItem:Impl.IconLabelContent {
        control: control
        alignment: Qt.AlignLeft | Qt.AlignVCenter
        //color: (control.pressed && !control.checked && !control.sectionDelegate) ? LingmoUI.Theme.highlightedTextColor : LingmoUI.Theme.textColor
    }

    indicator: Impl.SwitchIndicator {
        control: control
        mirrored: !control.mirrored
    }

    background: Impl.DelegateBackground {
        control: control
    }
}
