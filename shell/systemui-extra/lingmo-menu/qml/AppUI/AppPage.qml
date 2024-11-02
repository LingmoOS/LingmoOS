/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

import QtQuick 2.12
import QtQuick.Layouts 1.12
import org.lingmo.menu.core 1.0

Item {
    property alias search: appPageSearch
    property alias content: appPageContent

    ColumnLayout {
        anchors.fill: parent
        anchors.topMargin: 12
        anchors.bottomMargin: 5
        spacing: 5

        AppPageSearch {
            id: appPageSearch
            Layout.fillWidth: true
            Layout.preferredHeight: 40
            focusToPageContent: appPageContent
        }

        AppPageContent {
            id: appPageContent
            Layout.fillWidth: true
            Layout.fillHeight: true
            activeFocusOnTab: false
        }
    }
}
