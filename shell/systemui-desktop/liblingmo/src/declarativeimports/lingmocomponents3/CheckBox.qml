/*
    SPDX-FileCopyrightText: 2016 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2022 ivan (@ratijas) tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Templates as T
import org.kde.lingmoui as LingmoUI
import "private" as Private

T.CheckBox {
    id: control

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding,
                            implicitIndicatorWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding,
                             implicitIndicatorHeight + topPadding + bottomPadding)

    baselineOffset: contentItem.y + contentItem.baselineOffset
    spacing: LingmoUI.Units.smallSpacing
    hoverEnabled: true

    // Keeping old default smallMedium size for compatibility
    // with UIs that currently expect that as the default size
    icon.width: LingmoUI.Units.iconSizes.smallMedium
    icon.height: LingmoUI.Units.iconSizes.smallMedium

    indicator: CheckIndicator {
        x: (control.text || control.icon.name || control.icon.source)
            ? (control.mirrored ? control.width - width - control.rightPadding : control.leftPadding)
            : control.leftPadding + Math.round((control.availableWidth - width) / 2)
        y: control.topPadding + Math.round((control.availableHeight - height) / 2)

        control: control
    }

    contentItem: Private.IconLabel {
        readonly property int effectiveIndicatorWidth: control.indicator && control.indicator.visible && control.indicator.width > 0
            ? control.indicator.width + control.spacing : 0

        mirrored: control.mirrored
        leftPadding: !control.mirrored ? effectiveIndicatorWidth : 0
        rightPadding: control.mirrored ? effectiveIndicatorWidth : 0

        font: control.font
        alignment: Qt.AlignLeft | Qt.AlignVCenter
        display: control.display
        spacing: control.spacing
        iconItem.implicitWidth: control.icon.width
        iconItem.implicitHeight: control.icon.height
        iconItem.source: control.icon.name || control.icon.source
        label.text: control.text

        Rectangle { // As long as we don't enable antialiasing, not rounding should be fine
            parent: control.contentItem.label
            width: Math.min(parent.width, parent.contentWidth)
            height: 1
            anchors.left: parent.left
            anchors.top: parent.bottom
            color: LingmoUI.Theme.highlightColor
            visible: control.visualFocus
        }
    }
}
