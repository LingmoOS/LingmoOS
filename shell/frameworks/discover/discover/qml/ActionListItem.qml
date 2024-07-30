/*
 *   SPDX-FileCopyrightText: 2015 Aleix Pol Gonzalez <aleixpol@blue-systems.com>
 *
 *   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.lingmoui as LingmoUI
import org.kde.lingmoui.delegates as KD

QQC2.ItemDelegate {
    id: item

    Layout.fillWidth: true

    highlighted: checked
    visible: enabled

    Keys.onEnterPressed: trigger()
    Keys.onReturnPressed: trigger()

    function trigger() {
        if (enabled) {
            if (typeof drawer !== "undefined") {
                drawer.resetMenu()
            }
            action.trigger()
        }
    }

    property string subtitle
    property string stateIconName

    contentItem: RowLayout {
        spacing: LingmoUI.Units.largeSpacing
        KD.IconTitleSubtitle {
            Layout.fillWidth: true
            icon: icon.fromControlsIcon(item.icon)
            title: item.text
            subtitle: item.subtitle
            selected: item.highlighted
            font: item.font
        }
        LingmoUI.Icon {
            Layout.fillHeight: true
            visible: item.stateIconName.length > 0
            source: item.stateIconName
            implicitWidth: LingmoUI.Units.iconSizes.sizeForLabels
            implicitHeight: LingmoUI.Units.iconSizes.sizeForLabels
        }
    }

    LingmoUI.MnemonicData.enabled: item.enabled && item.visible
    LingmoUI.MnemonicData.controlType: LingmoUI.MnemonicData.MenuItem
    LingmoUI.MnemonicData.label: action.text

    // Note changing text here does not affect the action.text
    text: LingmoUI.MnemonicData.richTextLabel

    QQC2.ToolTip.text: shortcut.nativeText
    QQC2.ToolTip.visible: (LingmoUI.Settings.tabletMode ? down : hovered) && QQC2.ToolTip.text.length > 0
    QQC2.ToolTip.delay: LingmoUI.Units.toolTipDelay

    Shortcut {
        id: shortcut
        sequence: item.LingmoUI.MnemonicData.sequence
        onActivated: item.trigger()
    }

    // Using the generic onPressed so individual instances can override
    // behaviour using Keys.on{Up,Down}Pressed
    Keys.onPressed: event => {
        if (event.accepted) {
            return
        }

        // Using forceActiveFocus here since the item may be in a focus scope
        // and just setting focus won't focus the scope.
        if (event.key === Qt.Key_Up) {
            nextItemInFocusChain(false).forceActiveFocus()
            event.accepted = true
        } else if (event.key === Qt.Key_Down) {
            nextItemInFocusChain(true).forceActiveFocus()
            event.accepted = true
        }
    }
}
