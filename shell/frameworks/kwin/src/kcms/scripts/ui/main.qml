/*
    SPDX-FileCopyrightText: 2013 Antonis Tsiapaliokas <kok3rs@gmail.com>
    SPDX-FileCopyrightText: 2019 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts

import org.kde.config
import org.kde.lingmoui 2 as LingmoUI
import org.kde.newstuff as NewStuff
import org.kde.kcmutils as KCMUtils

KCMUtils.ScrollViewKCM {
    implicitWidth: LingmoUI.Units.gridUnit * 22
    implicitHeight: LingmoUI.Units.gridUnit * 20

    actions: [
        LingmoUI.Action {
            icon.name: "document-import"
            text: i18n("Install from File…")
            onTriggered: kcm.importScript()
        },
        NewStuff.Action {
            text: i18nc("@action:button get new KWin scripts", "Get New…")
            visible: KAuthorized.authorize(KAuthorized.GHNS)
            configFile: "kwinscripts.knsrc"
            onEntryEvent: (entry, event) => {
                if (event === NewStuff.Engine.StatusChangedEvent) {
                    kcm.onGHNSEntriesChanged()
                }
            }
        }
    ]

    header: ColumnLayout {
        spacing: LingmoUI.Units.smallSpacing

        LingmoUI.InlineMessage {
            Layout.fillWidth: true
            visible: kcm.errorMessage || kcm.infoMessage
            type: kcm.errorMessage ? LingmoUI.MessageType.Error : LingmoUI.MessageType.Information
            text: kcm.errorMessage || kcm.infoMessage
        }

        LingmoUI.SearchField {
            Layout.fillWidth: true
            id: searchField
        }
    }

    view: KCMUtils.PluginSelector {
        id: selector
        sourceModel: kcm.model
        query: searchField.text

        delegate: KCMUtils.PluginDelegate {
            onConfigTriggered: kcm.configure(model.config)
            additionalActions: [
                LingmoUI.Action {
                    enabled: kcm.canDeleteEntry(model.metaData)
                    icon.name: kcm.pendingDeletions.indexOf(model.metaData) === -1 ? "delete" : "edit-undo"
                    text: i18nc("@info:tooltip", "Delete…")
                    displayHint: LingmoUI.DisplayHint.IconOnly

                    onTriggered: kcm.togglePendingDeletion(model.metaData)
                }
            ]
        }
    }
}
