/*
    SPDX-FileCopyrightText: 2013 Kai Uwe Broulik <kde@privat.broulik.de>
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.20 as Kirigami
import org.kde.kitemmodels 1.0 as KItemModels

Kirigami.OverlaySheet {
    id: sheet

    readonly property alias view: sheetListView

    onOpened: {
        filter.text = "";
        filter.forceActiveFocus()
    }

    // Need to manually set the parent when using this in a Plasma config dialog
    parent: sheet.parent.parent

    header: ColumnLayout {
        Layout.preferredWidth: sheetListView.implicitWidth

        Kirigami.Heading {
            Layout.fillWidth: true
            text: i18n("Add More Dictionaries")
            textFormat: Text.PlainText
            wrapMode: Text.Wrap
        }
        Kirigami.SearchField {
            id: filter
            Layout.fillWidth: true
        }
    }

    footer: QQC2.DialogButtonBox {
        standardButtons: QQC2.DialogButtonBox.Ok
        onAccepted: sheet.close()
    }

    ListView {
        id: sheetListView
        focus: true // keyboard navigation
        activeFocusOnTab: true // keyboard navigation
        implicitWidth: Kirigami.Units.gridUnit * 25

        reuseItems: true

        model: KItemModels.KSortFilterProxyModel {
            sourceModel: dictionariesModel
            filterRoleName: "EditRole" // id
            filterString: filter.text
            filterCaseSensitivity: Qt.CaseInsensitive
        }

        delegate: QQC2.CheckDelegate {
            id: checkbox
            width: sheetListView.width
            focus: true // keyboard navigation
            text: `${model.id} (${model.description})`
            checked: model.checked
            onToggled: {
                model.checked = checked;
                sheetListView.currentIndex = index; // highlight
                sheetListView.forceActiveFocus(); // keyboard navigation
            }
            highlighted: ListView.isCurrentItem
        }
    }
}
