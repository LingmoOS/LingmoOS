/*
 * Copyright (C) 2024, KylinSoft Co., Ltd.
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

import QtQuick 2.0
import QtQml.Models 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.5
import org.lingmo.menu.core 1.0
import org.lingmo.quick.platform 1.0 as Platform
import org.lingmo.quick.items 1.0 as LingmoUIItems

SwipeView {
    id: folderSwipeView
    anchors.margins: contentMargins
    property bool needTurnPage: false
    property int contentX: contentItem.contentX
    property int startContentX: 0
    property int startIndex: 0

    property var folderContentModel
    property alias folderSwipeView: folderSwipeView
    property bool mouseEnable: false
    property real contentMargins
    property real labelMagrins
    property real labelSpacing
    property bool isFullScreen: false

    function normalScreenIconSignal() {
        turnPageFinished();
    }

    function hideFolder() {
        // 在非第一页时，执行翻页动画
        if (currentIndex > 0) {
            interactive = false;
            needTurnPage = true;
            pageIndicator.visible = false;
            contentItem.highlightFollowsCurrentItem = true;
            contentItem.highlightMoveDuration = 300;
            contentItem.highlightMoveVelocity = -1;
            startContentX = contentX;
            startIndex = currentIndex;
            currentIndex = 0;
            // 在第一页或单页时，全屏文件夹直接隐藏
        } else {
            reset();
        }
    }

    function reset() {
        interactive = true;
        root.active = false;
        pageIndicator.visible = false;
        turnPageFinished();
    }

    // 判断执行翻页结束后，全屏文件夹隐藏
    onContentXChanged: {
        // 对比翻页过程的位移
        if (needTurnPage && startContentX - contentX === startIndex * width) {
            needTurnPage = false;
            reset();
        }
    }

    Repeater {
        model: Math.ceil(folderModel.count / 16)

        Item {
            id: base
            property int currentPage: SwipeView.index

            GridView {
                id: folderGridView
                cellHeight: isFullScreen ? cellWidth : (cellWidth + 12)
                cellWidth: width / 4
                anchors.fill: parent
                interactive: false

                model: DelegateModel {
                    property int maxItemNumPerPage: 16
                    filterOnGroup: "folderContent"
                    groups: DelegateModelGroup {
                        name: "folderContent"
                    }

                    items.onChanged: {
                        var groupIndex = groups.length - 1;
                        if (groups[groupIndex].count !== 0) {
                            groups[groupIndex].remove(0, groups[groupIndex].count);
                        }

                        for (var i = base.currentPage * maxItemNumPerPage;
                             i < Math.min((base.currentPage + 1) * maxItemNumPerPage, items.count); i ++) {
                            items.addGroups(i, 1, "folderContent");
                        }
                    }
                    onCountChanged: {
                        if (count === 0) {
                            reset();
                            folderLoader.isFolderOpened = false;
                        }
                    }

                    model: folderContentModel
                    delegate: Item {
                        width: folderGridView.cellWidth
                        height: folderGridView.cellHeight

                        MouseArea {
                            hoverEnabled: true
                            enabled: mouseEnable
                            anchors.fill: parent
                            anchors.margins: labelMagrins
                            acceptedButtons: Qt.LeftButton | Qt.RightButton

                            LingmoUIItems.StyleBackground {
                                anchors.fill: parent
                                useStyleTransparency: false
                                paletteRole: Platform.Theme.Text
                                radius: Platform.Theme.maxRadius
                                alpha: parent.containsPress ? 0.15 : parent.containsMouse ? 0.08 : 0.00
                                clip: false

                                ColumnLayout {
                                    anchors.fill: parent
                                    anchors.margins: labelMagrins
                                    spacing: labelSpacing
                                    LingmoUIItems.Icon {
                                        id: iconImage
                                        source: icon
                                        Layout.minimumHeight: 16
                                        Layout.minimumWidth: 16
                                        Layout.maximumWidth: 96
                                        Layout.maximumHeight: 96

                                        Layout.fillHeight: true
                                        Layout.preferredWidth: height
                                        Layout.alignment: Qt.AlignHCenter
                                    }

                                    LingmoUIItems.StyleText {
                                        text: name
                                        elide: Text.ElideRight
                                        paletteRole: Platform.Theme.Text

                                        Layout.preferredHeight: contentHeight
                                        Layout.fillWidth: true
                                        horizontalAlignment: Text.AlignHCenter
                                    }
                                }
                            }
                            onClicked: {
                                if (mouse.button === Qt.RightButton) {
                                    menuManager.showMenu(id, MenuInfo.Folder);
                                    return;
                                }
                                if (mouse.button === Qt.LeftButton) {
                                    appManager.launchApp(id);
                                    return;
                                }
                            }
                        }

                        EditModeFlag {
                            isFavorited: true
                            anchors.top: parent.top
                            anchors.right: parent.right
                            anchors.rightMargin: 10
                            onClicked: {
                                extensionData.favoriteAppsModel.removeAppFromFavorites(id);
                            }
                        }
                    }
                }
            }
        }
    }
}
