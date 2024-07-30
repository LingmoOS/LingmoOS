/* SPDX-FileCopyrightText: 2017 The Qt Company Ltd.
 * SPDX-FileCopyrightText: 2017 Marco Martin <mart@kde.org>
 * SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LicenseRef-KDE-Accepted-LGPL
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Templates as T
import QtQuick.Controls as Controls
import org.kde.lingmoui as LingmoUI
import org.kde.ocean.impl as Impl

T.MenuItem {
    id: control

    // We can't do `control: control` inside a Component due to scoping issues, so set up an alias
    // to be used by the radio/check indicator components.
    property alias __controlRoot: control

    property bool __reserveSpaceForIndicator: menu?.__hasIndicators ?? false
    property bool __reserveSpaceForIcon: menu?.__hasIcons ?? false
    property bool __reserveSpaceForArrow: menu?.__hasArrows ?? false

    LingmoUI.Theme.colorSet: control.down || control.highlighted ? LingmoUI.Theme.Button : -1
    LingmoUI.Theme.inherit: !background || !background.visible && !(control.highlighted || control.down)

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding,
                            implicitIndicatorWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding,
                             implicitIndicatorHeight + topPadding + bottomPadding)
    baselineOffset: contentItem.y + contentItem.baselineOffset

//     width: parent ? parent.width : implicitWidth

    // Note: Binding height here to make sure menu items that are not visible are
    // properly collapsed, otherwise they will still occupy space inside the menu.
    height: visible ? undefined : 0

    icon.width: LingmoUI.Units.iconSizes.sizeForLabels
    icon.height: LingmoUI.Units.iconSizes.sizeForLabels

    Layout.fillWidth: true

    spacing: LingmoUI.Units.mediumSpacing
    padding: LingmoUI.Units.mediumSpacing
    leftPadding: {
        if (!control.indicator.visible
            && ((!contentItem.hasIcon && contentItem.textBesideIcon) // False if contentItem has been replaced
                || display == T.AbstractButton.TextOnly
                || display == T.AbstractButton.TextUnderIcon)) {
            return Impl.Units.mediumHorizontalPadding
        } else {
            return control.horizontalPadding
        }
    }
    rightPadding: {
        if (!control.arrow.visible
            && contentItem.hasLabel // False if contentItem has been replaced
            && display != T.AbstractButton.IconOnly) {
            return Impl.Units.mediumHorizontalPadding
        } else {
            return control.horizontalPadding
        }
    }

    LingmoUI.MnemonicData.enabled: control.enabled && control.visible
    LingmoUI.MnemonicData.controlType: LingmoUI.MnemonicData.MenuItem
    LingmoUI.MnemonicData.label: control.text
    Shortcut {
        //in case of explicit & the button manages it by itself
        enabled: !(RegExp(/\&[^\&]/).test(control.text))
        sequence: control.LingmoUI.MnemonicData.sequence
        onActivated: {
            if (control.checkable) {
                control.toggle();
            } else {
                control.clicked();
            }
        }
    }

    indicator: Loader {
        visible: control.checkable
        active: visible
        sourceComponent: control.autoExclusive ? radioIndicator : checkIndicator

        Component {
            id: checkIndicator
            Impl.CheckIndicator {
                control: __controlRoot
                checkState: control.checked ? Qt.Checked : Qt.Unchecked
            }
        }
        Component {
            id: radioIndicator
            Impl.RadioIndicator {
                control: __controlRoot
            }
        }
    }

    arrow: LingmoUI.Icon {
        anchors {
            right: control.right
            rightMargin: control.rightPadding
            verticalCenter: control.verticalCenter
        }
        source: control.mirrored ? "arrow-left" : "arrow-right"
        implicitWidth: LingmoUI.Units.iconSizes.sizeForLabels
        implicitHeight: LingmoUI.Units.iconSizes.sizeForLabels
        visible: control.subMenu
    }

    contentItem: Impl.IconLabelShortcutContent {
        control: control
        text: control.LingmoUI.MnemonicData.richTextLabel
        alignment: Qt.AlignLeft | Qt.AlignVCenter
        reserveSpaceForIndicator: control.__reserveSpaceForIndicator
        reserveSpaceForIcon: control.__reserveSpaceForIcon
        reserveSpaceForArrow: control.__reserveSpaceForArrow
    }

    background: Impl.MenuItemBackground {
        control: control
    }
}
