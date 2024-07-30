/*
    SPDX-FileCopyrightText: 2017 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2017 The Qt Company Ltd.

    SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-or-later
*/


import QtQuick
import QtQuick.Templates as T
import org.kde.kirigami as Kirigami

T.MenuBarItem {
    id: controlRoot

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding,
                             implicitIndicatorHeight + topPadding + bottomPadding)

    topPadding: Kirigami.Units.smallSpacing
    leftPadding: Kirigami.Units.largeSpacing
    rightPadding: Kirigami.Units.largeSpacing
    bottomPadding: Kirigami.Units.smallSpacing
    hoverEnabled: true

    Kirigami.MnemonicData.enabled: enabled && visible
    Kirigami.MnemonicData.controlType: Kirigami.MnemonicData.MenuItem
    Kirigami.MnemonicData.label: text

    Shortcut {
        //in case of explicit & the button manages it by itself
        enabled: !(RegExp(/\&[^\&]/).test(controlRoot.text))
        sequence: controlRoot.Kirigami.MnemonicData.sequence
        onActivated: controlRoot.clicked();
    }

    contentItem: Label {
        text: controlRoot.Kirigami.MnemonicData.richTextLabel
        font: controlRoot.font
        color: controlRoot.hovered && !controlRoot.pressed ? Kirigami.Theme.highlightedTextColor : Kirigami.Theme.textColor
        elide: Text.ElideRight
        visible: controlRoot.text
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }

    background: Rectangle {
        implicitWidth: 40
        implicitHeight: Kirigami.Units.gridUnit + 2 * Kirigami.Units.smallSpacing
        color: Kirigami.Theme.highlightColor
        opacity: controlRoot.down || controlRoot.highlighted ? 0.7 : 0

        Behavior on opacity {
            enabled: Kirigami.Units.shortDuration > 0
            NumberAnimation {
                duration: Kirigami.Units.shortDuration
            }
        }
    }
}
