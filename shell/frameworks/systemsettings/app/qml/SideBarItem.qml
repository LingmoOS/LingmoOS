/*
   SPDX-FileCopyrightText: 2017 Marco Martin <mart@kde.org>
   SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>

   SPDX-License-Identifier: LGPL-2.0-only
*/

import QtQuick

import org.kde.lingmoui as LingmoUI

LingmoUI.ApplicationItem {
    id: root

    implicitWidth: wideScreen ? LingmoUI.Units.gridUnit * 30 :  LingmoUI.Units.gridUnit * 15
    wideScreen: pageStack.depth > 1 && systemsettings.width > LingmoUI.Units.gridUnit * 70

    pageStack.initialPage: mainColumn
    pageStack.defaultColumnWidth: wideScreen ? root.width / 2 : root.width

    property alias searchMode: mainColumn.searchMode
    readonly property real headerHeight: mainColumn.header.height

    CategoriesPage {
        id: mainColumn
        focus: true
    }

    SubCategoryPage {
        id: subCategoryColumn
    }
}
