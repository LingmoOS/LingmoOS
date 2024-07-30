/*
    SPDX-FileCopyrightText: 2013-2015 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import org.kde.lingmo.components as LingmoComponents
import org.kde.lingmoui as LingmoUI

/**
 * Internal type containing the default layout of a tooltip.
 */
Item {
    property Item toolTip
    property int preferredTextWidth: LingmoUI.Units.gridUnit * 20

    implicitWidth: mainLayout.implicitWidth + LingmoUI.Units.largeSpacing * 2
    implicitHeight: mainLayout.implicitHeight + LingmoUI.Units.largeSpacing * 2

    LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft
    LayoutMirroring.childrenInherit: true

    LingmoUI.Theme.colorSet: LingmoUI.Theme.Window
    LingmoUI.Theme.inherit: false

    RowLayout {
        id: mainLayout
        anchors.centerIn: parent
        anchors.margins: LingmoUI.Units.largeSpacing

        spacing: LingmoUI.Units.gridUnit

        Image {
            source: toolTip ? toolTip.image : ""
            visible: toolTip !== null && toolTip.image !== ""
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
        }

        LingmoUI.Icon {
            animated: false
            source: toolTip ? toolTip.icon : ""
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
            visible: toolTip !== null && toolTip.icon !== "" && toolTip.image === "" && valid
            Layout.preferredWidth: LingmoUI.Units.iconSizes.medium
            Layout.preferredHeight: LingmoUI.Units.iconSizes.medium
        }

        ColumnLayout {
            Layout.maximumWidth: preferredTextWidth
            spacing: 0

            LingmoUI.Heading {
                level: 3
                Layout.fillWidth: true
                elide: Text.ElideRight
                wrapMode: Text.Wrap
                text: toolTip ? toolTip.mainText : ""
                textFormat: Text.PlainText
                visible: text !== ""
            }

            LingmoComponents.Label {
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                text: toolTip ? toolTip.subText : ""
                textFormat: toolTip ? toolTip.textFormat : Text.AutoText
                opacity: 0.6
                visible: text !== ""
                maximumLineCount: 8
            }
        }
    }
}
