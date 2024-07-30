/*
 * SPDX-FileCopyrightText: 2018 Friedrich W. H. Kossebau <kossebau@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick

import QtQuick.Layouts
import QtQuick.Controls as QQC2

import org.kde.lingmo.plasmoid
import org.kde.lingmo.extras as LingmoExtras
import org.kde.lingmo.core as LingmoCore
import org.kde.lingmoui as LingmoUI
import org.kde.lingmo.components as LingmoComponents

import org.kde.lingmo.private.weather

ColumnLayout {
    id: fullRoot

    Layout.margins: LingmoUI.Units.smallSpacing

    property alias generalModel: topPanel.generalModel
    property alias observationModel: topPanel.observationModel

    Layout.minimumWidth: Math.max(LingmoUI.Units.gridUnit * 10, implicitWidth)
    Layout.minimumHeight: Math.max(LingmoUI.Units.gridUnit * 10, implicitHeight)

    LingmoExtras.PlaceholderMessage {
        Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
        Layout.margins: LingmoUI.Units.gridUnit
        // when not in panel, a configure button is already shown for needsConfiguration
        visible: (root.status === Util.NeedsConfiguration) && (Plasmoid.formFactor === LingmoCore.Types.Vertical || Plasmoid.formFactor === LingmoCore.Types.Horizontal)
        iconName: "mark-location"
        text: i18n("Please set your location")
        helpfulAction: QQC2.Action {
            icon.name: "configure"
            text: i18n("Set location…")
            onTriggered: {
                Plasmoid.internalAction("configure").trigger();
            }
        }
    }

    LingmoExtras.PlaceholderMessage {
        Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
        Layout.margins: LingmoUI.Units.largeSpacing * 4
        Layout.maximumWidth: LingmoUI.Units.gridUnit * 20
        visible: root.status === Util.Timeout
        iconName: "network-disconnect"
        text: {
            const sourceTokens = root.weatherSource.split("|");
            return i18n("Unable to retrieve weather information for %1", sourceTokens[2]);
        }
        explanation: i18nc("@info:usagetip", "The network request timed out, possibly due to a server outage at the weather station provider. Check again later.")
    }

    TopPanel {
        id: topPanel
        visible: root.status !== Util.NeedsConfiguration && root.status !== Util.Timeout

        Layout.fillWidth: true
    }

    SwitchPanel {
        id: switchPanel
        visible: root.status === Util.Normal
        Layout.fillWidth: true

        forecastViewTitle: generalModel.forecastTitle
        forecastViewNightRow: generalModel.forecastNightRow
        forecastModel: root.forecastModel
        detailsModel: root.detailsModel
        noticesModel: root.noticesModel
    }

    LingmoComponents.Label {
        id: sourceLabel
        visible: root.status === Util.Normal
        readonly property string creditUrl: generalModel.creditUrl

        Layout.alignment: Qt.AlignVCenter | Qt.AlignRight

        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            acceptedButtons: Qt.NoButton
            cursorShape: !!parent.creditUrl ? Qt.PointingHandCursor : Qt.ArrowCursor
        }

        wrapMode: Text.WordWrap
        horizontalAlignment: Text.AlignRight
        font {
            pointSize: LingmoUI.Theme.smallFont.pointSize
            underline: !!creditUrl
        }
        linkColor : color
        opacity: 0.6
        textFormat: Text.StyledText

        text: {
            let result = generalModel.courtesy;
            if (creditUrl) {
                result = "<a href=\"" + creditUrl + "\">" + result + "</a>";
            }
            return result;
        }

        onLinkActivated: link => {
            Qt.openUrlExternally(link);
        }
    }
}
