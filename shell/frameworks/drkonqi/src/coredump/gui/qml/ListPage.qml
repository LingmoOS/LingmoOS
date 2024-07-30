// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2020-2022 Harald Sitter <sitter@kde.org>

import QtQuick 2.15
import QtQuick.Controls as QQC2
import QtQuick.Layouts 1.15
import org.kde.lingmoui 2.19 as LingmoUI
import org.kde.kitemmodels 1.0 as KItemModels

import org.kde.drkonqi.coredump.gui 1.0 as DrKonqi

LingmoUI.ScrollablePage {
    id: page
    title: i18nc("@title", "Crashes")

    actions: [
        LingmoUI.Action {
            displayComponent: LingmoUI.SearchField {
                onAccepted: patientFilterModel.filterString = text
            }
        }
    ]

    ListView {
        id: view
        reuseItems: true // We have a lot of items potentially, recycle them

        KItemModels.KSortFilterProxyModel { // set as model during state change
            id: patientFilterModel
            sourceModel: DrKonqi.PatientModel
            filterRoleName: "ROLE_appName"
            sortRoleName: "modelIndex"
            sortOrder: Qt.DescendingOrder
        }

        delegate: QQC2.ItemDelegate {
            id: delegate

            text: modelObject.appName
            icon.name: modelObject.iconName

            width: ListView.view.width
            onClicked: pageStack.push("qrc:/DetailsPage.qml", {patient: modelObject})

            contentItem: LingmoUI.IconTitleSubtitle {
                title: delegate.text
                subtitle: modelObject.dateTime
                icon: icon.fromControlsIcon(delegate.icon)
            }
        }

        LingmoUI.PlaceholderMessage {
            anchors.centerIn: parent
            width: parent.width - (LingmoUI.Units.largeSpacing * 4)
            visible: page.state === "loading"
            icon.name: "search"
            text: i18nc("@info place holder for empty listview", "Loading crash reports")
        }

        LingmoUI.PlaceholderMessage {
            anchors.centerIn: parent
            width: parent.width - (LingmoUI.Units.largeSpacing * 4)
            visible: page.state === "noData"
            icon.name: "emblem-checked"
            text: i18nc("@info place holder for empty listview", "No processes have crashed yet")
        }

        LingmoUI.PlaceholderMessage {
            anchors.centerIn: parent
            width: parent.width - (LingmoUI.Units.largeSpacing * 4)
            visible: page.state === "badSearch"
            icon.name: "search"
            text: i18nc("@info place holder for empty listview", "No crashes matching the search")
        }
    }

    states: [
        State {
            name: "loading"
            when: !DrKonqi.PatientModel.ready
        },
        State {
            name: "noData"
            when: DrKonqi.PatientModel.count === 0
        },
        State {
            name: "badSearch"
            when: patientFilterModel.count === 0
        },
        State {
            name: "" // default state
            PropertyChanges { target: view; model: patientFilterModel }
        }
    ]
}
