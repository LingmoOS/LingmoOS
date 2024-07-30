/*
    SPDX-FileCopyrightText: 2012-2013 Daniel Nicoletti <dantti12@gmail.com>
    SPDX-FileCopyrightText: 2014-2015 Jan Grulich <jgrulich@redhat.com>
    SPDX-FileCopyrightText: 2020 Nate Graham <nate@kde.org>
    SPDX-FileCopyrightText: 2023 Mike Noe <noeerover@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import org.kde.lingmo.extras as LingmoExtras
import org.kde.lingmoui as LingmoUI
import org.kde.lingmo.printmanager as PrintManager

LingmoExtras.ExpandableListItem {
    readonly property bool isPaused: model.printerState === 5

    icon: model.iconName
    iconEmblem: isPaused ? "emblem-pause" : ""
    title: model.info + (model.location && printersModel.displayLocationHint
            ? " (%1)".arg(model.location)
            : "")
    subtitle: model.stateMessage
    isDefault: model.isDefault

    defaultActionButtonAction: LingmoUI.Action {
        icon.name: isPaused ? "media-playback-start" : "media-playback-pause"
        text: isPaused ? i18n("Resume") : i18n("Pause")

        onTriggered: {
            if (isPaused) {
                printersModel.resumePrinter(model.printerName);
            } else {
                printersModel.pausePrinter(model.printerName);
            }
        }
    }

    contextualActions: [
        LingmoUI.Action {
            icon.name: "configure"
            text: i18n("Configure printer…")
            onTriggered: PrintManager.ProcessRunner.configurePrinter(model.printerName);
        },
        LingmoUI.Action {
            icon.name: "view-list-details"
            text: i18n("View print queue…")
            onTriggered: PrintManager.ProcessRunner.openPrintQueue(model.printerName);
        }
    ]
}
