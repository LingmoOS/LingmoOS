/*
    SPDX-FileCopyrightText: 2016 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2022 ivan (@ratijas) tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Templates as T
import org.kde.lingmoui as LingmoUI
import "private" as Private

T.Switch {
    id: control

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding,
                            implicitIndicatorWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding,
                             implicitIndicatorHeight + topPadding + bottomPadding)

    baselineOffset: contentItem.y + contentItem.baselineOffset
    hoverEnabled: true

    spacing: LingmoUI.Units.smallSpacing

    icon.width: LingmoUI.Units.iconSizes.sizeForLabels
    icon.height: LingmoUI.Units.iconSizes.sizeForLabels

    indicator: SwitchIndicator {
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
    }
}
