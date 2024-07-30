/*
 * SPDX-FileCopyrightText: 2014 Joseph Wenninger <jowenn@kde.org>
 * SPDX-FileCopyrightText: 2018 Piotr Kąkol <piotrkakol@protonmail.com>
 *
 * Based on fuzzy-clock main.qml:
 * SPDX-FileCopyrightText: 2013 Heena Mahour <heena393@gmail.com>
 * SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>
 * SPDX-FileCopyrightText: 2014 Kai Uwe Broulik <kde@privat.broulik.de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQml
import QtQuick 2.0
import QtQuick.Layouts 1.1

import org.kde.lingmo.plasmoid 2.0
import org.kde.lingmoui 2.20 as LingmoUI
import org.kde.lingmo.lingmo5support 2.0 as P5Support
import org.kde.lingmo.core as LingmoCore
import org.kde.lingmo.extras 2.0 as LingmoExtras
import org.kde.lingmo.workspace.calendar 2.0 as LingmoCalendar

PlasmoidItem {
    id: root

    property bool showSeconds: plasmoid.configuration.showSeconds
    property int hours
    property int minutes
    property int seconds
    width: LingmoUI.Units.gridUnit * 10
    height: LingmoUI.Units.gridUnit * 4

    Plasmoid.backgroundHints: LingmoCore.Types.DefaultBackground | LingmoCore.Types.ConfigurableBackground
    preferredRepresentation: compactRepresentation

    // keep this consistent with toolTipMainText and toolTipSubText in analog-clock
    toolTipMainText: Qt.locale().toString(dataSource.data["Local"]["DateTime"],"dddd")
    toolTipSubText: Qt.locale().toString(dataSource.data["Local"]["DateTime"], Qt.locale().timeFormat(Locale.LongFormat)) + "\n" + Qt.locale().toString(dataSource.data["Local"]["DateTime"], Qt.locale().dateFormat(Locale.LongFormat).replace(/(^dddd.?\s)|(,?\sdddd$)/, ""))

    P5Support.DataSource {
        id: dataSource
        engine: "time"
        connectedSources: ["Local"]
        intervalAlignment: plasmoid.configuration.showSeconds || compactRepresentationItem.mouseArea.containsMouse ? P5Support.Types.NoAlignment : P5Support.Types.AlignToMinute
        interval: showSeconds || compactRepresentationItem.mouseArea.containsMouse ? 1000 : 60000

        onDataChanged: {
            var date = new Date(data["Local"]["DateTime"]);
            hours = date.getHours();
            minutes = date.getMinutes();
            seconds = date.getSeconds();
        }
        Component.onCompleted: {
            onDataChanged();
        }
    }

    compactRepresentation: BinaryClock { }

    fullRepresentation: LingmoCalendar.MonthView {
        Layout.minimumWidth: LingmoUI.Units.gridUnit * 20
        Layout.minimumHeight: LingmoUI.Units.gridUnit * 20

        today: dataSource.data["Local"]["DateTime"]
    }
}
