/*
    SPDX-FileCopyrightText: 2012-2013 Daniel Nicoletti <dantti12@gmail.com>
    SPDX-FileCopyrightText: 2014-2015 Jan Grulich <jgrulich@redhat.com>
    SPDX-FileCopyrightText: 2021 Carl Schwan <carl@carlschwan.eu>
    SPDX-FileCopyrightText: 2023 Mike Noe <noeerover@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import org.kde.lingmo.extras as LingmoExtras
import org.kde.lingmo.components as LingmoComponents3
import org.kde.lingmoui as LingmoUI
import org.kde.kitemmodels as KItemModels
import org.kde.lingmo.printmanager as PrintManager

LingmoExtras.Representation {
    collapseMarginsHint: true

    header: LingmoExtras.PlasmoidHeading {
        LingmoExtras.SearchField {
            anchors.fill: parent
            onTextChanged: printersFilterModel.filterString = text.toLowerCase()
        }
    }

    LingmoComponents3.ScrollView {
        anchors.fill: parent
        contentWidth: availableWidth - contentItem.leftMargin - contentItem.rightMargin

        contentItem: ListView {
            focus: true
            currentIndex: -1

            section {
                property: printersModel.printersOnly ? "" : "isClass"
                delegate: LingmoUI.ListSectionHeader {
                    width: ListView.view.width
                    required property bool section
                    label: !section ? i18n("Printers") : i18n("Printer Groups")
                }
            }
            
            model: KItemModels.KSortFilterProxyModel {
                id: printersFilterModel
                sourceModel: printersModel
                sortRoleName: "isClass"
                
                filterRowCallback: (source_row, source_parent) => {
                    return sourceModel.data(sourceModel.index(source_row, 0, source_parent), PrintManager.PrinterModel.DestDescription).toLowerCase().includes(filterString)
                }

            }

            topMargin: LingmoUI.Units.smallSpacing * 2
            bottomMargin: LingmoUI.Units.smallSpacing * 2
            leftMargin: LingmoUI.Units.smallSpacing * 2
            rightMargin: LingmoUI.Units.smallSpacing * 2
            spacing: LingmoUI.Units.smallSpacing

            highlight: LingmoExtras.Highlight {}
            highlightMoveDuration: LingmoUI.Units.shortDuration
            highlightResizeDuration: LingmoUI.Units.shortDuration
            delegate: PrinterDelegate {}

            Loader {
                anchors.centerIn: parent
                width: parent.width - (LingmoUI.Units.largeSpacing * 4)
                active: printersFilterModel.count === 0 || serverUnavailable
                sourceComponent: LingmoExtras.PlaceholderMessage {
                    text: serverUnavailable ? printersModelError || i18n("No printers have been configured or discovered") : i18n("No matches")
                    iconName: serverUnavailable ? "dialog-error" : "edit-none"
                }
            }
        }
    }
}
