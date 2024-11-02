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
import QtQuick.Controls 2.12

import org.lingmo.menu.core 1.0
import org.lingmo.menu.utils 1.0
import org.lingmo.quick.platform 1.0 as Platform
import org.lingmo.quick.items 1.0 as LingmoUIItems

LingmoUIItems.StyleBackground {
    useStyleTransparency: false
    paletteRole: Platform.Theme.Text
    radius: childrenRect.width / 2
    alpha: 0.06

    RowLayout {
        id: pluginSortButtonRoot
        property int currentId: 0
        anchors.fill: parent
        anchors.topMargin: 2
        anchors.bottomMargin: 2
        anchors.leftMargin: 2
        anchors.rightMargin: 2
        // property var model: appPageHeaderUtils.model(PluginGroup.SortMenuItem)
        spacing: 4

        LingmoUIItems.Button {
            id: pluginLetterSortButton

            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.maximumWidth: height
            activeFocusOnTab: true
            ToolTip.delay: 500
            ToolTip.text: qsTr("Letter Sort")
            ToolTip.visible: containsMouse

            background.radius: width / 2
            icon.mode: parent.currentId === 0 ? LingmoUIItems.Icon.AutoHighlight : LingmoUIItems.Icon.Disabled
            icon.source: pluginSortButtonRoot.model.getProviderIcon(0);
            background.paletteRole: Platform.Theme.Light
            background.alpha: parent.currentId === 0 ? 0.7 : 0

            onClicked: {
                pluginSortButtonRoot.model.changeProvider(0);
            }
            Keys.onPressed: {
                if (event.key === Qt.Key_Enter || event.key === Qt.Key_Return) {
                    pluginSortButtonRoot.model.changeProvider(0);
                }
            }
        }

        LingmoUIItems.Button {
            id: pluginCateGoryButton

            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.maximumWidth: height
            activeFocusOnTab: true
            ToolTip.delay: 500
            ToolTip.text: qsTr("Category")
            ToolTip.visible: containsMouse

            background.radius: width / 2
            icon.mode: parent.currentId === 1 ? LingmoUIItems.Icon.AutoHighlight : LingmoUIItems.Icon.Disabled
            icon.source: pluginSortButtonRoot.model.getProviderIcon(1);
            background.paletteRole: Platform.Theme.Light
            background.alpha: parent.currentId === 1 ? 0.7 : 0

            onClicked: {
                pluginSortButtonRoot.model.changeProvider(1);
            }
            Keys.onPressed: {
                if (event.key === Qt.Key_Enter || event.key === Qt.Key_Return) {
                    pluginSortButtonRoot.model.changeProvider(1);
                }
            }
        }

        Component.onCompleted: {
            updateProviderId();
            pluginSortButtonRoot.model.currentIndexChanged.connect(updateProviderId);
        }

        Component.onDestruction: {
            pluginSortButtonRoot.model.currentIndexChanged.disconnect(updateProviderId);
        }

        function updateProviderId() {
            currentId = pluginSortButtonRoot.model.currentProviderId();
        }
    }
}
