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

import org.kde.lingmo.plasmoid 2.0
import org.kde.lingmo.core as LingmoCore
import org.kde.lingmoui 2.20 as LingmoUI
import org.kde.lingmo.lingmo5support 2.0 as P5Support
import org.kde.lingmo.extras 2.0 as LingmoExtras
import org.kde.lingmo.workspace.calendar 2.0 as LingmoCalendar

PlasmoidItem {
    id: root

    readonly property date currentDateTime: dataSource.data.Local ? dataSource.data.Local.DateTime : new Date()

    width: LingmoUI.Units.gridUnit * 10
    height: LingmoUI.Units.gridUnit * 4

    preferredRepresentation: compactRepresentation

    // keep this consistent with toolTipMainText and toolTipSubText in analog-clock
    toolTipMainText: Qt.locale().toString(currentDateTime, "dddd")
    toolTipSubText: Qt.locale().toString(currentDateTime, Qt.locale().timeFormat(Locale.LongFormat)) + "\n" + Qt.locale().toString(currentDateTime, Qt.locale().dateFormat(Locale.LongFormat).replace(/(^dddd.?\s)|(,?\sdddd$)/, ""))

    Plasmoid.backgroundHints: LingmoCore.Types.ShadowBackground | LingmoCore.Types.ConfigurableBackground

    P5Support.DataSource {
        id: dataSource
        engine: "time"
        connectedSources: ["Local"]
        interval: root.compactRepresentationItem.mouseArea.containsMouse ? 1000 : 60000
        intervalAlignment: root.compactRepresentationItem.mouseArea.containsMouse ? P5Support.Types.NoAlignment : P5Support.Types.AlignToMinute
    }

    compactRepresentation: FuzzyClock { }

    fullRepresentation: LingmoCalendar.MonthView {
        Layout.minimumWidth: LingmoUI.Units.gridUnit * 20
        Layout.minimumHeight: LingmoUI.Units.gridUnit * 20

        LingmoUI.Theme.inherit: false
        LingmoUI.Theme.colorSet: LingmoUI.Theme.Window

        today: currentDateTime
    }
}
