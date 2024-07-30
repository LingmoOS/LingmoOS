/*
    SPDX-FileCopyrightText: 2019 Carson Black <uhhadd@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.5
import QtQuick.Controls 2.5 as QQC2
import QtQuick.Layouts 1.0

import org.kde.lingmoui 2.8 as LingmoUI

LingmoUI.GlobalDrawer {
    id: root
    edge: Qt.RightEdge

    handleOpenIcon.source: "dialog-close"
    handleClosedIcon.source: "view-preview"

    focus: false

    actions: cuttlefish.actions

    LingmoUI.Heading {
        level: 1
        Layout.fillWidth: true
        wrapMode: Text.Wrap
        horizontalAlignment: Text.AlignHCenter
        elide: Text.ElideRight
        text: preview.iconName
    }
    LingmoUI.FormLayout {
        Layout.fillWidth: true
        Layout.topMargin: LingmoUI.Units.largeSpacing
        QQC2.Label {
            Layout.maximumWidth: LingmoUI.Units.gridUnit * 10
            LingmoUI.FormData.label: i18n("File name:")
            elide: Text.ElideRight
            text: preview.fileName
        }
        QQC2.Label {
            LingmoUI.FormData.label: i18n("Category:")
            font.capitalization: Font.Capitalize
            text: preview.category
        }
        QQC2.Label {
            LingmoUI.FormData.label: i18n("Scalable:")
            text: preview.scalable ? i18n("yes") : i18n("no")
            font.capitalization: Font.Capitalize
        }
    }
    IconMontage {
        id: grid
        columns: cuttlefish.iconSizes.length - 1
        Layout.alignment: Qt.AlignHCenter
    }
}
