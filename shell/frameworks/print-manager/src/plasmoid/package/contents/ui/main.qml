/*
    SPDX-FileCopyrightText: 2012-2013 Daniel Nicoletti <dantti12@gmail.com>
    SPDX-FileCopyrightText: 2014 Jan Grulich <jgrulich@redhat.com>
    SPDX-FileCopyrightText: 2021 Nate Graham <nate@kde.org>
    SPDX-FileCopyrightText: 2023 Mike Noe <noeerover@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts // fullRepresentation sizing
import org.kde.lingmo.plasmoid
import org.kde.lingmo.core as LingmoCore
import org.kde.lingmoui as LingmoUI
import org.kde.config // KAuthorized
import org.kde.kcmutils // KCMLauncher
import org.kde.lingmo.printmanager as PrintManager

PlasmoidItem {

    readonly property bool inPanel: (Plasmoid.location === LingmoCore.Types.TopEdge
        || Plasmoid.location === LingmoCore.Types.RightEdge
        || Plasmoid.location === LingmoCore.Types.BottomEdge
        || Plasmoid.location === LingmoCore.Types.LeftEdge)

    property alias serverUnavailable: printersModel.serverUnavailable
    property string printersModelError: ""

    PrintManager.PrinterModel {
        id: printersModel
        onError: (lastError, errorTitle, errorMsg) => {
            printersModelError = errorTitle;
        }
    }

    PrintManager.JobSortFilterModel {
        id: activeJobsFilterModel

        sourceModel: PrintManager.JobModel {
            Component.onCompleted: setWhichJobs(PrintManager.JobModel.WhichActive)
        }
    }

    toolTipMainText: i18n("Printers")
    toolTipSubText: {
        if (serverUnavailable && printersModelError) {
            return printersModelError;
        } else if (activeJobsFilterModel.activeCount > 1) {
            return i18np("There is one print job in the queue",
                         "There are %1 print jobs in the queue",
                         activeJobsFilterModel.activeCount);
        // If there is only one job, show more information about it
        } else if (activeJobsFilterModel.activeCount === 1) {
            const idx = activeJobsFilterModel.index(0, 0);
            const jobName = activeJobsFilterModel.data(idx, PrintManager.JobModel.RoleJobName);
            const printerName = activeJobsFilterModel.data(idx, PrintManager.JobModel.RoleJobPrinter);
            if (jobName) {
                return i18nc("Printing document name with printer name", "Printing %1 with %2", jobName, printerName);
            } else {
                return printerName === "" ? "" : i18nc("Printing with printer name", "Printing with %1", printerName);
            }
        } else if (printersModel.count > 0) {
            return i18n("Print queue is empty");
        } else {
            return i18n("No printers have been configured or discovered");
        }
    }

    Plasmoid.icon: inPanel ? "printer-symbolic" : "printer"

    fullRepresentation: FullRepresentation {
        focus: true
        // as a desktop widget, we need to start with a reasonable size
        Layout.preferredWidth: inPanel ? -1 : LingmoUI.Units.gridUnit * 24
        Layout.preferredHeight: inPanel ? -1 : LingmoUI.Units.gridUnit * 24
    }

    switchWidth: LingmoUI.Units.gridUnit * 10
    switchHeight: LingmoUI.Units.gridUnit * 10

    Plasmoid.status: {
        if (activeJobsFilterModel.activeCount > 0) {
            return LingmoCore.Types.ActiveStatus;
        } else if (printersModel.count > 0 || serverUnavailable) {
            return LingmoCore.Types.PassiveStatus;
        } else {
            return LingmoCore.Types.HiddenStatus;
        }
    }

    // Overwrite default configure menu item
    LingmoCore.Action {
        id: configAction
        text: i18n("&Configure Printers…")
        icon.name: "configure"
        shortcut: "alt+d,s"
        enabled: KAuthorized.authorizeControlModule("kcm_printer_manager")
        onTriggered: KCMLauncher.openSystemSettings("kcm_printer_manager")
    }

    Component.onCompleted: Plasmoid.setInternalAction("configure", configAction)
}
