/*
    SPDX-FileCopyrightText: 2014 Martin Klapetek <mklapetek@kde.org>
    SPDX-FileCopyrightText: 2019 Kai Uwe Broulik <kde@broulik.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts

import org.kde.lingmo.components as LingmoComponents3
import org.kde.lingmo.extras as LingmoExtras
import org.kde.lingmoui as LingmoUI

RowLayout {
    id: root

    // OSD Timeout in msecs - how long it will stay on the screen
    property int timeout: 1800
    // This is either a text or a number, if showingProgress is set to true,
    // the number will be used as a value for the progress bar
    property var osdValue
    // Maximum percent value
    property int osdMaxValue: 100
    // Icon name to display
    property string icon
    // Set to true if the value is meant for progress bar,
    // false for displaying the value as normal text
    property bool showingProgress: false

    function formatPercent(number) {
        return i18ndc("lingmo_lookandfeel_org.kde.lookandfeel", "Percentage value", "%1%", number);
    }

    spacing: LingmoUI.Units.largeSpacing

    Layout.preferredWidth: Math.max(Math.min(Screen.desktopAvailableWidth / 2, implicitWidth), LingmoUI.Units.gridUnit * 15)
    Layout.preferredHeight: LingmoUI.Units.iconSizes.medium
    Layout.minimumWidth: Layout.preferredWidth
    Layout.minimumHeight: Layout.preferredHeight
    Layout.maximumWidth: Layout.preferredWidth
    Layout.maximumHeight: Layout.preferredHeight
    width: Layout.preferredWidth
    height: Layout.preferredHeight

    LingmoUI.Icon {
        id: iconItem
        Layout.leftMargin: LingmoUI.Units.smallSpacing // Left end spacing
        Layout.preferredWidth: LingmoUI.Units.iconSizes.medium
        Layout.preferredHeight: LingmoUI.Units.iconSizes.medium
        Layout.alignment: Qt.AlignVCenter
        source: root.icon
        visible: valid
    }

    LingmoComponents3.ProgressBar {
        id: progressBar
        Layout.leftMargin: iconItem.valid ? 0 : LingmoUI.Units.smallSpacing // Left end spacing
        Layout.fillWidth: true
        Layout.alignment: Qt.AlignVCenter
        // So it never exceeds the minimum popup size
        Layout.minimumWidth: 0
        Layout.rightMargin: LingmoUI.Units.smallSpacing
        visible: root.showingProgress
        from: 0
        to: root.osdMaxValue
        value: Number(root.osdValue)
    }

    // Get the width of a three-digit number so we can size the label
    // to the maximum width to avoid the progress bad resizing itself
    TextMetrics {
        id: widestLabelSize
        text: formatPercent(root.osdMaxValue)
        font: percentageLabel.font
    }

    // Numerical display of progress bar value
    LingmoExtras.Heading {
        id: percentageLabel
        Layout.rightMargin: LingmoUI.Units.smallSpacing // Right end spacing
        Layout.fillHeight: true
        Layout.preferredWidth: Math.ceil(widestLabelSize.advanceWidth)
        Layout.alignment: Qt.AlignVCenter
        level: 3
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        text: formatPercent(progressBar.value)
        textFormat: Text.PlainText
        wrapMode: Text.NoWrap
        visible: root.showingProgress
        // Display a subtle visual indication that the volume might be
        // dangerously high
        // ------------------------------------------------
        // Keep this in sync with the copies in lingmo-pa:ListItemBase.qml
        // and lingmo-pa:VolumeSlider.qml
        color: {
            if (progressBar.value <= 100) {
                return LingmoUI.Theme.textColor
            } else if (progressBar.value > 100 && progressBar.value <= 125) {
                return LingmoUI.Theme.neutralTextColor
            } else {
                return LingmoUI.Theme.negativeTextColor
            }
        }
    }

    LingmoExtras.Heading {
        Layout.fillWidth: true
        Layout.fillHeight: true
        Layout.leftMargin: iconItem.valid ? 0 : LingmoUI.Units.smallSpacing
        Layout.rightMargin: LingmoUI.Units.smallSpacing
        Layout.alignment: Qt.AlignVCenter
        level: 3
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        textFormat: Text.PlainText
        wrapMode: Text.NoWrap
        elide: Text.ElideRight
        text: !root.showingProgress && root.osdValue ? root.osdValue : ""
        visible: !root.showingProgress
    }
}
