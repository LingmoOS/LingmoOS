/*
    SPDX-FileCopyrightText: 2014-2017 Sebastian Kügler <sebas@kde.org>
    SPDX-FileCopyrightText: 2019 Carson Black <uhhadd@gmail.com>
    SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.2
import QtQuick.Controls 2.5 as QQC2
import QtQuick.Layouts 1.0
import QtQuick.Dialogs

import org.kde.lingmoui 2.20 as LingmoUI

QQC2.ScrollView {
    id: root

    property alias iconPreview: iconPreview

    LingmoUI.Theme.colorSet: LingmoUI.Theme.Window

    background: Rectangle {
        color: LingmoUI.Theme.backgroundColor
    }

    ColumnLayout {
        width: root.availableWidth
        spacing: LingmoUI.Units.largeSpacing

        LingmoUI.SelectableLabel {
            Accessible.role: Accessible.Heading
            Layout.fillWidth: true

            Layout.margins: LingmoUI.Units.largeSpacing
            Layout.topMargin: LingmoUI.Units.gridUnit * 2

            // Match `level: 1` in LingmoUI.Heading
            font.pointSize: LingmoUI.Theme.defaultFont.pointSize * 1.35
            // Match `type: LingmoUI.Heading.Type.Primary` in LingmoUI.Heading
            font.weight: Font.DemiBold
            horizontalAlignment: Text.AlignHCenter
            text: preview.iconName
        }
        LingmoUI.FormLayout {
            Layout.fillWidth: true
            Layout.margins: LingmoUI.Units.largeSpacing
            Layout.topMargin: 0

            QQC2.Label {
                LingmoUI.FormData.label: i18n("File name:")
                Layout.fillWidth: true
                elide: Text.ElideMiddle
                text: preview.fileName
            }
            QQC2.Label {
                LingmoUI.FormData.label: i18n("Category:")
                Layout.fillWidth: true
                font.capitalization: Font.Capitalize
                text: preview.category
            }
            QQC2.Label {
                LingmoUI.FormData.label: i18n("Scalable:")
                Layout.fillWidth: true
                text: preview.scalable ? i18n("yes") : i18n("no")
                font.capitalization: Font.Capitalize
            }
        }
        IconMontage {
            id: iconPreview
            columns: cuttlefish.iconSizes.length - 1
            Layout.fillWidth: true
            Layout.topMargin: LingmoUI.Units.largeSpacing
            Layout.bottomMargin: LingmoUI.Units.largeSpacing * 4
        }
        Repeater {
            model: cuttlefish.actions
            delegate: QQC2.Button {
                Layout.alignment: Qt.AlignHCenter
                action: modelData
            }
        }
        Item {
            // Keep it to apply ColumnLayout::spacing as a bottom margin.
            Layout.fillHeight: true
        }
    }
}
