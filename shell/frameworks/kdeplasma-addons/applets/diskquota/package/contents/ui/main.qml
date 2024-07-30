/*
 * SPDX-FileCopyrightText: 2015 Dominik Haumann <dhaumann@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
import QtQuick 2.1
import QtQuick.Layouts 1.1

import org.kde.plasma.core as PlasmaCore
import org.kde.kirigami 2.20 as Kirigami
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.plasma.extras 2.0 as PlasmaExtras

import org.kde.plasma.private.diskquota 1.0

PlasmoidItem {
    id: quotaApplet

    Layout.minimumWidth: Kirigami.Units.gridUnit * 10
    Layout.minimumHeight: Kirigami.Units.gridUnit * 2

    Plasmoid.status: {
        switch (diskQuota.status) {
            case DiskQuota.NeedsAttentionStatus:
                return PlasmaCore.Types.NeedsAttentionStatus
            case DiskQuota.ActiveStatus:
                return PlasmaCore.Types.ActiveStatus
        }
        // default case: DiskQuota.PassiveStatus
        return PlasmaCore.Types.PassiveStatus
    }

    switchWidth: Kirigami.Units.gridUnit * 10
    switchHeight: Kirigami.Units.gridUnit * 10

    Plasmoid.icon: diskQuota.iconName
    toolTipMainText: diskQuota.toolTip
    toolTipSubText: diskQuota.subToolTip

    Component.onCompleted: plasmoid.removeAction("configure")

    DiskQuota {
        id: diskQuota
    }

    fullRepresentation: Item {
        id: root

        width: Kirigami.Units.gridUnit * 20
        height: Kirigami.Units.gridUnit * 14

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
            width: parent.width - Kirigami.Units.gridUnit * 4

            active: !diskQuota.quotaInstalled || listView.count == 0
            visible: active
            asynchronous: true

            sourceComponent: PlasmaExtras.PlaceholderMessage {
                width: parent.width
                readonly property bool hasText: model.filterRegularExpression.length > 0
                iconName: diskQuota.quotaInstalled ? "edit-none" : "disk-quota"
                text: diskQuota.quotaInstalled ? i18nc("@info:status", "No quota restrictions found") : i18nc("@info:status", "Quota tool not found")
                explanation: diskQuota.quotaInstalled ? "" : i18nc("@info:usagetip", "Please install 'quota'")
            }
        }

        PlasmaComponents3.ScrollView {
            anchors.fill: parent
            ListView {
                id: listView
                model: diskQuota.model
                boundsBehavior: Flickable.StopAtBounds
                highlight: PlasmaExtras.Highlight { }
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
