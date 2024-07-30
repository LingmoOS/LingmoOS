// SPDX-FileCopyrightText: 2017 Marco Martin <mart@kde.org>
// SPDX-FileCopyrightText: 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-or-later

import QtQuick
import QtQuick.Layouts
import QtQuick.Templates as T
import org.kde.lingmoui as LingmoUI

T.MenuBarItem {
    id: controlRoot

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding,
                             implicitIndicatorHeight + topPadding + bottomPadding)

    topPadding: LingmoUI.Units.smallSpacing
    leftPadding: LingmoUI.Units.largeSpacing
    rightPadding: LingmoUI.Units.largeSpacing
    bottomPadding: LingmoUI.Units.smallSpacing
    hoverEnabled: true

    LingmoUI.MnemonicData.enabled: enabled && visible
    LingmoUI.MnemonicData.controlType: LingmoUI.MnemonicData.MenuItem
    LingmoUI.MnemonicData.label: text

    Shortcut {
        //in case of explicit & the button manages it by itself
        enabled: !(RegExp(/\&[^\&]/).test(controlRoot.text))
        sequence: controlRoot.LingmoUI.MnemonicData.sequence
        onActivated: controlRoot.clicked();
    }

    contentItem: Label {
        text: controlRoot.LingmoUI.MnemonicData.richTextLabel
        font: controlRoot.font
        color: controlRoot.hovered && !controlRoot.pressed ? LingmoUI.Theme.highlightedTextColor : LingmoUI.Theme.textColor
        elide: Text.ElideRight
        visible: controlRoot.text
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }

    background: Rectangle {
        implicitWidth: 40
        implicitHeight: LingmoUI.Units.gridUnit + 2 * LingmoUI.Units.smallSpacing
        color: LingmoUI.Theme.highlightColor
        opacity: controlRoot.down || controlRoot.highlighted ? 0.7 : 0

        Behavior on opacity {
            NumberAnimation {
                duration: LingmoUI.Units.shortDuration
            }
        }
    }
}
