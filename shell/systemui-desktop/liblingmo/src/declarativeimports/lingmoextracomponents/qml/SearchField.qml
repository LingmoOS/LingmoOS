// SPDX-FileCopyrightText: 2019 Carl-Lucien Schwan <carl@carlschwan.eu>
// SPDX-FileCopyrightText: 2022 Felipe Kinoshita <kinofhek@gmail.com>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QQC2

import org.kde.lingmo.components as LingmoComponents3
import org.kde.lingmo.extras as LingmoExtras
import org.kde.ksvg as KSvg
import org.kde.lingmoui as LingmoUI

/**
 * This is a standard textfield following KDE HIG. Using Ctrl+F as focus
 * sequence and "Search…" as placeholder text.
 *
 * Example usage for the search field component:
 * @code
 * import org.kde.lingmo.extras as LingmoExtras
 *
 * LingmoExtras.SearchField {
 *     id: searchField
 *     onAccepted: console.log("Search text is " + searchField.text)
 * }
 * @endcode
 *
 * @inherit org::lingmos::extras::ActionTextField
 * @since 5.93
 */
LingmoExtras.ActionTextField {
    id: root

    property int _iconWidth: searchIcon.width + searchIcon.anchors.leftMargin

    // padding to accommodate search icon nicely
    leftPadding: if (root.effectiveHorizontalAlignment === TextInput.AlignRight) {
        return (_rightActionsRow.visible ? _rightActionsRow.width : 0) + (__hasBackgroundAndMargins ? background.margins.left : 0);
    } else {
        return _iconWidth + (_leftActionsRow.visible ? _leftActionsRow.width : 0) + (__hasBackgroundAndMargins ? background.margins.left : 0);
    }
    rightPadding: if (root.effectiveHorizontalAlignment === TextInput.AlignRight) {
        return _iconWidth + (_leftActionsRow.visible ? _leftActionsRow.width : 0) + (__hasBackgroundAndMargins ? background.margins.right : 0);
    } else {
        return (_rightActionsRow.visible ? _rightActionsRow.width : 0) + (__hasBackgroundAndMargins ? background.margins.right : 0);
    }

    LingmoUI.Icon {
        id: searchIcon
        LayoutMirroring.enabled: root.effectiveHorizontalAlignment === TextInput.AlignRight
        anchors.left: root.left
        anchors.leftMargin: LingmoUI.Units.smallSpacing * 2
        anchors.verticalCenter: root.verticalCenter
        anchors.verticalCenterOffset: Math.round((root.topPadding - root.bottomPadding) / 2)
        implicitHeight: LingmoUI.Units.iconSizes.sizeForLabels
        implicitWidth: LingmoUI.Units.iconSizes.sizeForLabels

        source: "search"
    }

    placeholderText: i18nd("liblingmo6", "Search…")

    Accessible.name: i18nd("liblingmo6", "Search")
    Accessible.searchEdit: true

    focusSequence: StandardKey.Find
    inputMethodHints: Qt.ImhNoPredictiveText

    rightActions: [
        LingmoUI.Action {
            //ltr confusingly refers to the direction of the arrow in the icon, not the text direction which it should be used in
            icon.name: root.effectiveHorizontalAlignment === TextInput.AlignRight ? "edit-clear-locationbar-ltr" : "edit-clear-locationbar-rtl"
            visible: root.text.length > 0
            text: i18nd("liblingmo6", "Clear search")
            onTriggered: {
                root.clear();
                root.accepted();
            }
        }
    ]
}
