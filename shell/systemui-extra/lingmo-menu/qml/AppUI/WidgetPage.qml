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

import QtQml 2.12
import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.5

import AppControls2 1.0 as AppControls2
import org.lingmo.quick.platform 1.0 as Platform
import org.lingmo.quick.items 1.0 as LingmoUIItems
import org.lingmo.menu.core 1.0
import org.lingmo.menu.utils 1.0
import org.lingmo.menu.extension 1.0

Item {
    ColumnLayout {
        anchors.fill: parent
        anchors.topMargin: 12
        anchors.bottomMargin: 5
        spacing: 5

        Item {
            // header
            Layout.fillWidth: true
            Layout.preferredHeight: 40
            height: 40

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 12
                anchors.rightMargin:12
                spacing: 0

                ListView {
                    id: widgetList
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    clip: true
                    spacing: 24
                    interactive: false
                    orientation: ListView.Horizontal
                    function send(data) {
                        if (currentItem !== null) {
                            model.send(currentIndex, data);
                        }
                    }
                    onCurrentIndexChanged: {
                        if (currentItem !== null) {
                            currentItem.select();
                        }
                    }
                    model: WidgetModel {}
                    delegate: Component {
                        // 插件信息
                        LingmoUIItems.StyleBackground {
                            useStyleTransparency: false
                            paletteRole: Platform.Theme.Highlight
                            alpha: 0
                            radius: Platform.Theme.minRadius
                            borderColor: Platform.Theme.Highlight
                            border.width: activeFocus ? 2 : 0

                            property var extensionData: model.data
                            property var extensionOptions: model.options
                            width: styleText.width
                            height: ListView.view ? ListView.view.height : 0

                            activeFocusOnTab: true
                            KeyNavigation.down: widgetLoader
                            Keys.onPressed: {
                                if (event.key === Qt.Key_Enter || event.key === Qt.Key_Return) {
                                    widgetList.currentIndex = model.index;
                                    EventTrack.sendClickEvent("switch_plugin", "Sidebar", {"plugin": model.name});
                                }
                            }

                            onExtensionDataChanged: {
                                if (widgetLoader.source === model.main) {
                                    widgetLoader.item.extensionData = extensionData;
                                }
                            }

                            function select() {
                                if (widgetLoader.source !== model.main) {
                                    widgetLoader.setSource(model.main, {extensionData: extensionData});
                                }
                            }

                            LingmoUIItems.StyleText {
                                height: parent.height
                                id: styleText
                                verticalAlignment: Text.AlignVCenter
                                horizontalAlignment: Text.AlignHCenter
                                font.bold: parent.ListView.isCurrentItem

                                paletteRole: parent.ListView.isCurrentItem ? Platform.Theme.Highlight: Platform.Theme.Text
                                text: model.name
                            }

                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    widgetList.currentIndex = model.index;
                                    EventTrack.sendClickEvent("switch_plugin", "Sidebar", {"plugin": model.name});
                                }
                            }
                        }
                    }
                }

                Loader {
                    id: widgetMenuLoader
                    Layout.preferredWidth: 34
                    Layout.preferredHeight: 34
                    Layout.alignment: Qt.AlignVCenter
                }
            }
        }

        Loader {
            id: widgetLoader
            // 加载插件区域
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true

            onLoaded: {
                item.send.connect(widgetList.send);
                // sidebarLayout.updateSidebarLayout(widgetList.currentItem.extensionOptions);
                updateMenu();
                item.extensionMenuChanged.connect(updateMenu);
            }
            Keys.onTabPressed: {
                widgetList.focus = true
            }

            function updateMenu() {
                if (item === null) {
                    return;
                }
                if (item.extensionMenu !== null) {
                    widgetMenuLoader.sourceComponent = item.extensionMenu;
                } else {
                    widgetMenuLoader.sourceComponent = undefined;
                }
            }
        }
    }

    Component.onCompleted: {
        if (widgetList.count > 0) {
            widgetList.currentIndex = 0;
        }
    }
}
