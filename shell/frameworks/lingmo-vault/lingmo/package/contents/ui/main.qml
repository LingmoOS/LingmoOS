/*
 *   SPDX-FileCopyrightText: 2017, 2018, 2019 Ivan Cukic <ivan.cukic (at) kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.lingmoui as LingmoUI
import org.kde.kitemmodels as KItemModels
import org.kde.lingmo.components as LingmoComponents3
import org.kde.lingmo.core as LingmoCore
import org.kde.lingmo.extras as LingmoExtras
import org.kde.lingmo.plasmoid

PlasmoidItem {
    readonly property var vaultsModelActions: Plasmoid.vaultsModel.actionsModel()

    readonly property bool inPanel: [
        LingmoCore.Types.TopEdge,
        LingmoCore.Types.RightEdge,
        LingmoCore.Types.BottomEdge,
        LingmoCore.Types.LeftEdge,
    ].includes(Plasmoid.location)

    Plasmoid.busy: vaultsModelActions.isBusy

    Plasmoid.icon: {
        let iconName = (vaultsModelActions.hasError ? "lingmovault_error" : "lingmovault");

        if (inPanel) {
            return iconName += "-symbolic"
        }

        return iconName;
    }

    Plasmoid.status: vaultsModelActions.hasOpenVaults ? LingmoCore.Types.ActiveStatus : LingmoCore.Types.PassiveStatus

    onExpandedChanged: {
        Plasmoid.vaultsModel.reloadDevices();
    }

    Plasmoid.contextualActions: [
        LingmoCore.Action {
            id: createAction
            text: i18nd("lingmovault-kde", "Create a New Vault…")
            icon.name: "list-add-symbolic"
            onTriggered: checked => root.vaultsModelActions.requestNewVault()
        }
    ]

    fullRepresentation: LingmoExtras.Representation {

        Layout.minimumWidth: LingmoUI.Units.gridUnit * 18
        Layout.minimumHeight: LingmoUI.Units.gridUnit * 12

        collapseMarginsHint: true

        LingmoComponents3.ScrollView {
            anchors.fill: parent

            // HACK: workaround for https://bugreports.qt.io/browse/QTBUG-83890
            LingmoComponents3.ScrollBar.horizontal.policy: LingmoComponents3.ScrollBar.AlwaysOff

            contentWidth: availableWidth - vaultsList.leftMargin - vaultsList.rightMargin

            contentItem: ListView {
                id: vaultsList

                model: KItemModels.KSortFilterProxyModel {
                    sortRoleName: "name"
                    sourceModel: Plasmoid.vaultsModel
                }

                clip: true
                currentIndex: -1
                spacing: LingmoUI.Units.smallSpacing
                topMargin: LingmoUI.Units.largeSpacing
                leftMargin: LingmoUI.Units.largeSpacing
                rightMargin: LingmoUI.Units.largeSpacing
                bottomMargin: LingmoUI.Units.largeSpacing

                highlight: LingmoExtras.Highlight {}
                highlightMoveDuration: LingmoUI.Units.shortDuration
                highlightResizeDuration: LingmoUI.Units.shortDuration
                delegate: VaultItem {}

                LingmoExtras.PlaceholderMessage {
                    id: noVaultsMessage

                    anchors.centerIn: parent
                    width: parent.width - (LingmoUI.Units.gridUnit * 4)

                    visible: vaultsList.count === 0

                    iconName: "lingmovault"
                    text: i18nd("lingmovault-kde", "No Vaults have been set up")

                    helpfulAction: QQC2.Action {
                        text: createAction.text
                        icon.name: createAction.icon.name

                        onTriggered: source => createAction.trigger()
                    }
                }
            }
        }

        footer: LingmoExtras.PlasmoidHeading {
            visible: vaultsList.count > 0 && !(Plasmoid.containmentDisplayHints & LingmoCore.Types.ContainmentDrawsPlasmoidHeading)

            contentItem: RowLayout {
                spacing: LingmoUI.Units.smallSpacing

                LingmoComponents3.ToolButton {

                    text: createAction.text
                    icon.name: createAction.icon.name

                    onClicked: createAction.trigger()
                    Layout.alignment: Qt.AlignLeft
                }
            }
        }
    }
}
