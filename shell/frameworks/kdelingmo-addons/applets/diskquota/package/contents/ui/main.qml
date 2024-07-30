/*
 * SPDX-FileCopyrightText: 2015 Dominik Haumann <dhaumann@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
import QtQuick 2.1
import QtQuick.Layouts 1.1

import org.kde.lingmo.core as LingmoCore
import org.kde.lingmoui 2.20 as LingmoUI
import org.kde.lingmo.plasmoid 2.0
import org.kde.lingmo.components 3.0 as LingmoComponents3
import org.kde.lingmo.extras 2.0 as LingmoExtras

import org.kde.lingmo.private.diskquota 1.0

PlasmoidItem {
    id: quotaApplet

    Layout.minimumWidth: LingmoUI.Units.gridUnit * 10
    Layout.minimumHeight: LingmoUI.Units.gridUnit * 2

    Plasmoid.status: {
        switch (diskQuota.status) {
            case DiskQuota.NeedsAttentionStatus:
                return LingmoCore.Types.NeedsAttentionStatus
            case DiskQuota.ActiveStatus:
                return LingmoCore.Types.ActiveStatus
        }
        // default case: DiskQuota.PassiveStatus
        return LingmoCore.Types.PassiveStatus
    }

    switchWidth: LingmoUI.Units.gridUnit * 10
    switchHeight: LingmoUI.Units.gridUnit * 10

    Plasmoid.icon: diskQuota.iconName
    toolTipMainText: diskQuota.toolTip
    toolTipSubText: diskQuota.subToolTip

    Component.onCompleted: plasmoid.removeAction("configure")

    DiskQuota {
        id: diskQuota
    }

    fullRepresentation: Item {
        id: root

        width: LingmoUI.Units.gridUnit * 20
        height: LingmoUI.Units.gridUnit * 14

        // HACK: connection to reset currentIndex to -1. Without this, when
        // uninstalling filelight, the selection highlight remains fixed (which is wrong)
        Connections {
            target: diskQuota
            function onCleanUpToolInstalledChanged() {
                if (!diskQuota.cleanUpToolInstalled) {
                    listView.currentIndex = -1
                }
            }
        }

        Loader {
            id: emptyHint

            anchors.centerIn: parent
            width: parent.width - LingmoUI.Units.gridUnit * 4

            active: !diskQuota.quotaInstalled || listView.count == 0
            visible: active
            asynchronous: true

            sourceComponent: LingmoExtras.PlaceholderMessage {
                width: parent.width
                readonly property bool hasText: model.filterRegularExpression.length > 0
                iconName: diskQuota.quotaInstalled ? "edit-none" : "disk-quota"
                text: diskQuota.quotaInstalled ? i18nc("@info:status", "No quota restrictions found") : i18nc("@info:status", "Quota tool not found")
                explanation: diskQuota.quotaInstalled ? "" : i18nc("@info:usagetip", "Please install 'quota'")
            }
        }

        LingmoComponents3.ScrollView {
            anchors.fill: parent
            ListView {
                id: listView
                model: diskQuota.model
                boundsBehavior: Flickable.StopAtBounds
                highlight: LingmoExtras.Highlight { }
                highlightMoveDuration: 0
                highlightResizeDuration: 0
                currentIndex: -1
                delegate: ListDelegateItem {
                    enabled: diskQuota.cleanUpToolInstalled
                    width: listView.width
                    mountPoint: model.mountPoint
                    details: model.details
                    iconName: model.icon
                    usedString: model.used
                    freeString: model.free
                    usage: model.usage
                }
            }
        }
    }
}
