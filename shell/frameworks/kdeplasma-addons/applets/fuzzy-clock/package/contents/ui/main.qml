/*
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
import org.kde.plasma.core as PlasmaCore
import org.kde.kirigami 2.20 as Kirigami
import org.kde.plasma.plasma5support 2.0 as P5Support
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.plasma.workspace.calendar 2.0 as PlasmaCalendar

PlasmoidItem {
    id: root

    readonly property date currentDateTime: dataSource.data.Local ? dataSource.data.Local.DateTime : new Date()

    width: Kirigami.Units.gridUnit * 10
    height: Kirigami.Units.gridUnit * 4

    preferredRepresentation: compactRepresentation

    // keep this consistent with toolTipMainText and toolTipSubText in analog-clock
    toolTipMainText: Qt.locale().toString(currentDateTime, "dddd")
    toolTipSubText: Qt.locale().toString(currentDateTime, Qt.locale().timeFormat(Locale.LongFormat)) + "\n" + Qt.locale().toString(currentDateTime, Qt.locale().dateFormat(Locale.LongFormat).replace(/(^dddd.?\s)|(,?\sdddd$)/, ""))

    Plasmoid.backgroundHints: PlasmaCore.Types.ShadowBackground | PlasmaCore.Types.ConfigurableBackground

    P5Support.DataSource {
        id: dataSource
        engine: "time"
        connectedSources: ["Local"]
        interval: root.compactRepresentationItem.mouseArea.containsMouse ? 1000 : 60000
        intervalAlignment: root.compactRepresentationItem.mouseArea.containsMouse ? P5Support.Types.NoAlignment : P5Support.Types.AlignToMinute
    }

    compactRepresentation: FuzzyClock { }

    fullRepresentation: PlasmaCalendar.MonthView {
        Layout.minimumWidth: Kirigami.Units.gridUnit * 20
        Layout.minimumHeight: Kirigami.Units.gridUnit * 20

        Kirigami.Theme.inherit: false
        Kirigami.Theme.colorSet: Kirigami.Theme.Window

        today: currentDateTime
    }
}
