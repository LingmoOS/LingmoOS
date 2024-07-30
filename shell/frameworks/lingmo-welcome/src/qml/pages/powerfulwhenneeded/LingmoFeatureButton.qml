/*
 *   SPDX-FileCopyrightText: 2022 Nate Graham <nate@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2

import org.kde.lingmoui as LingmoUI

QQC2.Button {
    id: root

    required property string title
    required property string subtitle
    required property string buttonIcon
    readonly property int implicitTitleWidth: metrics.width
    readonly property int fixedSizeStuff: icon.implicitWidth + leftPadding + rightPadding + titleRowLayout.spacing

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding)

    leftPadding: LingmoUI.Units.largeSpacing
    rightPadding: LingmoUI.Units.largeSpacing
    topPadding: LingmoUI.Units.largeSpacing
    bottomPadding: LingmoUI.Units.largeSpacing

    TextMetrics {
        id: metrics
        text: root.title
    }

    contentItem: ColumnLayout {
        spacing: 0

        RowLayout {
            id: titleRowLayout

            spacing: LingmoUI.Units.smallSpacing
            Layout.minimumHeight: LingmoUI.Units.gridUnit * 2 + spacing

            // Icon
            LingmoUI.Icon {
                id: icon
                Layout.preferredWidth: LingmoUI.Units.iconSizes.medium
                Layout.preferredHeight: LingmoUI.Units.iconSizes.medium
                Layout.alignment: Qt.AlignVCenter
                source: root.buttonIcon
            }
            // Title
            LingmoUI.Heading {
                Layout.fillWidth: true
                level: 4
                text: root.title
                verticalAlignment: Text.AlignVCenter
                maximumLineCount: 2
                elide: Text.ElideRight
                wrapMode: Text.Wrap
            }
        }

        // Subtitle
        QQC2.Label {
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: root.subtitle
            text: root.subtitle
            elide: Text.ElideRight
            wrapMode: Text.Wrap
            opacity: 0.6
            verticalAlignment: Text.AlignTop
        }
    }
}
