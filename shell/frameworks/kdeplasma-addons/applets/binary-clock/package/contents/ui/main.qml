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

import org.kde.plasma.plasmoid 2.0
import org.kde.kirigami 2.20 as Kirigami
import org.kde.plasma.plasma5support 2.0 as P5Support
import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.plasma.workspace.calendar 2.0 as PlasmaCalendar

PlasmoidItem {
    id: root

    property bool showSeconds: plasmoid.configuration.showSeconds
    property int hours
    property int minutes
    property int seconds
    width: Kirigami.Units.gridUnit * 10
    height: Kirigami.Units.gridUnit * 4

    Plasmoid.backgroundHints: PlasmaCore.Types.DefaultBackground | PlasmaCore.Types.ConfigurableBackground
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

    fullRepresentation: PlasmaCalendar.MonthView {
        Layout.minimumWidth: Kirigami.Units.gridUnit * 20
        Layout.minimumHeight: Kirigami.Units.gridUnit * 20

        today: dataSource.data["Local"]["DateTime"]
    }
}
