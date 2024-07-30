/*
 * SPDX-FileCopyrightText: 2018 Friedrich W. H. Kossebau <kossebau@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick

import QtQuick.Layouts
import QtQuick.Controls as QQC2

import org.kde.plasma.plasmoid
import org.kde.plasma.extras as PlasmaExtras
import org.kde.plasma.core as PlasmaCore
import org.kde.kirigami as Kirigami
import org.kde.plasma.components as PlasmaComponents

import org.kde.plasma.private.weather

ColumnLayout {
    id: fullRoot

    Layout.margins: Kirigami.Units.smallSpacing

    property alias generalModel: topPanel.generalModel
    property alias observationModel: topPanel.observationModel

    Layout.minimumWidth: Math.max(Kirigami.Units.gridUnit * 10, implicitWidth)
    Layout.minimumHeight: Math.max(Kirigami.Units.gridUnit * 10, implicitHeight)

    PlasmaExtras.PlaceholderMessage {
        Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
        Layout.margins: Kirigami.Units.gridUnit
        // when not in panel, a configure button is already shown for needsConfiguration
        visible: (root.status === Util.NeedsConfiguration) && (Plasmoid.formFactor === PlasmaCore.Types.Vertical || Plasmoid.formFactor === PlasmaCore.Types.Horizontal)
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

    PlasmaExtras.PlaceholderMessage {
        Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
        Layout.margins: Kirigami.Units.largeSpacing * 4
        Layout.maximumWidth: Kirigami.Units.gridUnit * 20
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

    PlasmaComponents.Label {
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
            pointSize: Kirigami.Theme.smallFont.pointSize
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
