/*
    SPDX-FileCopyrightText: 2017 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2017 The Qt Company Ltd.

    SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-or-later
*/


import QtQuick
import QtQuick.Layouts
import QtQuick.Templates as T
import org.kde.lingmoui as LingmoUI
import org.kde.desktop.private as Private

T.SwitchDelegate {
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
    leftPadding: !mirrored ? horizontalPadding + implicitIndicatorWidth + spacing : horizontalPadding
    rightPadding: mirrored ? horizontalPadding + implicitIndicatorWidth + spacing : horizontalPadding

    icon.width: LingmoUI.Units.iconSizes.smallMedium
    icon.height: LingmoUI.Units.iconSizes.smallMedium

    T.ToolTip.visible: (LingmoUI.Settings.tabletMode ? down : hovered) && (contentItem.truncated ?? false)
    T.ToolTip.text: text
    T.ToolTip.delay: LingmoUI.Units.toolTipDelay

    leftInset: TableView.view ? 0 : horizontalPadding / 2
    rightInset: TableView.view ? 0 : horizontalPadding / 2
    topInset: TableView.view ? 0 : Math.ceil(verticalPadding / 2)
    bottomInset: TableView.view ? 0 : Math.ceil(verticalPadding / 2)

    contentItem: RowLayout {
        LayoutMirroring.enabled: controlRoot.mirrored
        spacing: controlRoot.spacing

        property alias truncated: textLabel.truncated

        LingmoUI.Icon {
            Layout.alignment: Qt.AlignVCenter
            visible: controlRoot.icon.name !== "" || controlRoot.icon.source.toString() !== ""
            source: controlRoot.icon.name !== "" ? controlRoot.icon.name : controlRoot.icon.source
            Layout.preferredHeight: controlRoot.icon.height
            Layout.preferredWidth: controlRoot.icon.width
        }

        Label {
            id: textLabel

            Layout.alignment: Qt.AlignLeft
            Layout.fillWidth: true

            text: controlRoot.text
            font: controlRoot.font
            color: (controlRoot.pressed && !controlRoot.checked && !controlRoot.sectionDelegate) ? LingmoUI.Theme.highlightedTextColor : LingmoUI.Theme.textColor
            elide: Text.ElideRight
            visible: controlRoot.text
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignVCenter
        }
    }

    indicator: Private.SwitchIndicator {
        x: !controlRoot.mirrored ? controlRoot.horizontalPadding : controlRoot.width - width - controlRoot.horizontalPadding
        y: controlRoot.topPadding + (controlRoot.availableHeight - height) / 2
        control: controlRoot
    }

    background: Private.DefaultListItemBackground {
        // This is intentional and ensures the inset is not directly applied to
        // the background, allowing it to determine how to handle the inset.
        anchors.fill: parent
        control: controlRoot
    }
}
