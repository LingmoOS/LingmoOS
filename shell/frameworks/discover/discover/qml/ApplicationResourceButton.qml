/*
 *   SPDX-FileCopyrightText: 2022 Nate Graham <nate@kde.org>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2

import org.kde.lingmoui as LingmoUI

QQC2.Button {
    id: root

    required property string buttonIcon
    required property string title
    required property string subtitle
    property string tooltipText
    readonly property int implicitMinWidth: leftPadding +
                                            layout.spacing +
                                            textMetrics.width +
                                            layout.spacing +
                                            icon.Layout.preferredWidth +
                                            layout.spacing +
                                            rightPadding

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding)

    leftPadding: LingmoUI.Units.largeSpacing
    rightPadding: LingmoUI.Units.largeSpacing
    topPadding: LingmoUI.Units.largeSpacing
    bottomPadding: LingmoUI.Units.largeSpacing

    TextMetrics {
        id: textMetrics
        text: root.title
    }

    contentItem: ColumnLayout {
        spacing: 0
        RowLayout {
            id: layout
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
    QQC2.ToolTip {
        text: root.tooltipText ? root.tooltipText : ""
    }
}
