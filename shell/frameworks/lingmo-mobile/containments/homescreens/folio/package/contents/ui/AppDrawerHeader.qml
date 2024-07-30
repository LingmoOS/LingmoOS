// SPDX-FileCopyrightText: 2021-2023 Devin Lin <devin@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.lingmoui as LingmoUI

import org.kde.lingmo.components 3.0 as LingmoComponents
import org.kde.private.mobile.homescreen.folio 1.0 as Folio
import './delegate'

Item {
    id: root
    property Folio.HomeScreen folio

    LingmoUI.Theme.colorSet: LingmoUI.Theme.Complementary
    LingmoUI.Theme.inherit: false

    function clearSearchText(): void {
        searchField.text = '';
    }

    RowLayout {
        anchors.topMargin: LingmoUI.Units.largeSpacing
        anchors.leftMargin: LingmoUI.Units.gridUnit + LingmoUI.Units.largeSpacing
        anchors.rightMargin: LingmoUI.Units.gridUnit + LingmoUI.Units.largeSpacing
        anchors.fill: parent

        LingmoUI.SearchField {
            id: searchField
            onTextChanged: folio.ApplicationListSearchModel.setFilterFixedString(text)
            Layout.maximumWidth: LingmoUI.Units.gridUnit * 30
            Layout.alignment: Qt.AlignHCenter

            background: Rectangle {
                radius: LingmoUI.Units.cornerRadius
                color: Qt.rgba(255, 255, 255, (searchField.hovered || searchField.focus) ? 0.2 : 0.1)

                Behavior on color { ColorAnimation {} }
            }

            LingmoUI.Theme.inherit: false
            LingmoUI.Theme.colorSet: LingmoUI.Theme.Complementary

            topPadding: LingmoUI.Units.largeSpacing + LingmoUI.Units.smallSpacing
            bottomPadding: LingmoUI.Units.largeSpacing + LingmoUI.Units.smallSpacing
            Layout.fillWidth: true

            horizontalAlignment: QQC2.TextField.AlignHCenter
            placeholderText: i18nc("@info:placeholder", "Search applications…")
            placeholderTextColor: Qt.rgba(255, 255, 255, 0.8)
            color: 'white'

            font.weight: Font.Bold

            Connections {
                target: folio.HomeScreenState
                function onViewStateChanged(): void {
                    if (folio.HomeScreenState.viewState !== Folio.HomeScreenState.AppDrawerView) {
                        // Reset search field if the app drawer is not shown
                        if (searchField.text !== '') {
                            searchField.text = '';
                        }
                    }
                }
            }
        }
    }
}
