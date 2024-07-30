/*
    SPDX-FileCopyrightText: 2017 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2017 The Qt Company Ltd.
    SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Templates as T
import org.kde.lingmoui as LingmoUI
import org.kde.desktop.private as Private

T.ItemDelegate {
    id: controlRoot

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding,
                             implicitIndicatorHeight + topPadding + bottomPadding)

    hoverEnabled: true

    spacing: LingmoUI.Units.smallSpacing
    padding: LingmoUI.Settings.tabletMode ? LingmoUI.Units.largeSpacing : LingmoUI.Units.mediumSpacing
    horizontalPadding: LingmoUI.Units.smallSpacing * 2
    leftPadding: !mirrored ? horizontalPadding + (indicator ? implicitIndicatorWidth + spacing : 0) : horizontalPadding
    rightPadding: mirrored ? horizontalPadding + (indicator ? implicitIndicatorWidth + spacing : 0) : horizontalPadding

    icon.width: LingmoUI.Units.iconSizes.smallMedium
    icon.height: LingmoUI.Units.iconSizes.smallMedium

    T.ToolTip.visible: (LingmoUI.Settings.tabletMode ? down : hovered) && (contentItem.truncated ?? false)
    T.ToolTip.text: text
    T.ToolTip.delay: LingmoUI.Units.toolTipDelay

    leftInset: TableView.view ? 0 : horizontalPadding / 2
    rightInset: TableView.view ? 0 : horizontalPadding / 2
    // We want total spacing between consecutive list items to be
    // verticalPadding. So use half that as top/bottom margin, separately
    // ceiling/flooring them so that the total spacing is preserved.
    topInset: TableView.view ? 0 : Math.ceil(verticalPadding / 2)
    bottomInset: TableView.view ? 0 : Math.ceil(verticalPadding / 2)

    contentItem: RowLayout {
        LayoutMirroring.enabled: controlRoot.mirrored
        spacing: controlRoot.spacing

        property alias truncated: textLabel.truncated

        LingmoUI.Icon {
            selected: controlRoot.highlighted || controlRoot.down
            Layout.alignment: Qt.AlignVCenter
            visible: controlRoot.icon.name.length > 0 || controlRoot.icon.source.toString().length > 0
            source: controlRoot.icon.name.length > 0 ? controlRoot.icon.name : controlRoot.icon.source
            Layout.preferredHeight: controlRoot.icon.height
            Layout.preferredWidth: controlRoot.icon.width
        }

        Label {
            id: textLabel

            Layout.alignment: Qt.AlignLeft
            Layout.fillWidth: true

            text: controlRoot.text
            font: controlRoot.font
            color: controlRoot.highlighted || controlRoot.down
                ? LingmoUI.Theme.highlightedTextColor
                : (controlRoot.enabled ? LingmoUI.Theme.textColor : LingmoUI.Theme.disabledTextColor)

            elide: Text.ElideRight
            visible: controlRoot.text
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignVCenter
        }
    }

    background: Private.DefaultListItemBackground {
        // This is intentional and ensures the inset is not directly applied to
        // the background, allowing it to determine how to handle the inset.
        anchors.fill: parent
        control: controlRoot
    }
}
