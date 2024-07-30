// SPDX-FileCopyrightText: 2023 Devin Lin <devin@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as Controls
import QtQuick.Effects

import org.kde.lingmoui 2.20 as LingmoUI

import org.kde.private.mobile.homescreen.folio 1.0 as Folio

Item {
    id: root
    property Folio.HomeScreen folio

    property Folio.FolioApplicationFolder folder

    property bool expandBackground: false

    height: folio.FolioSettings.delegateIconSize
    width: folio.FolioSettings.delegateIconSize

    Rectangle {
        id: rect
        radius: LingmoUI.Units.cornerRadius
        color: Qt.rgba(255, 255, 255, 0.3)
        anchors.fill: parent

        property real scaleAmount: root.expandBackground ? 1.2 : 1.0

        Behavior on scaleAmount { NumberAnimation { duration: LingmoUI.Units.longDuration; easing.type: Easing.InOutQuad } }

        transform: Scale {
            origin.x: root.width / 2
            origin.y: root.height / 2
            xScale: rect.scaleAmount
            yScale: rect.scaleAmount
        }
    }

    Grid {
        id: previewGrid
        anchors.fill: parent
        anchors.margins: LingmoUI.Units.smallSpacing * 2
        columns: 2
        spacing: LingmoUI.Units.smallSpacing

        property var previews: root.folder.appPreviews

        Repeater {
            model: previewGrid.previews
            delegate: LingmoUI.Icon {
                implicitWidth: Math.round((previewGrid.width - previewGrid.spacing) / 2)
                implicitHeight: Math.round((previewGrid.width - previewGrid.spacing) / 2)
                source: modelData.icon
            }
        }
    }
}

