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
import AppUI 1.0 as AppUI

Loader {
    id: root
    active: false
    property var folderModel
    property string folderName: ""
    property int folderX: 0
    property int folderY: 0
    property int viewMaxRow: 0
    property bool isFolderOpened: false

    property bool isFullScreen: true
    property int margins: isFullScreen? 20 : 0
    property int animationDuration: 300
    signal turnPageFinished()

    function initFolder(id, name, x, y) {
        extensionData.folderModel.setFolderId(id);
        folderName = name;
        folderX = x;
        folderY = y;
        viewMaxRow = Math.ceil(folderModel.count / 4) > 4 ? 4 : Math.ceil(folderModel.count / 4);
        active = true;
        isFolderOpened = true;
    }

    sourceComponent: folderComponent

    Component {
        id: folderComponent
        Item {
            LingmoUIItems.StyleBackground {
                id: folderIconBase
                paletteRole: Platform.Theme.Text
                useStyleTransparency: false
                property int folderIconSize: 0
                property int iconSpacing: 0
                property int imageX: 0
                property int imageY: 0
                property int gridViewMargin: 8

                state: isFolderOpened ? "folderOpened" : "folderHidden"
                states: [
                    State {
                        name: "folderOpened"
                        PropertyChanges {
                            target: folderIconBase
                            width: 348
                            height: viewMaxRow * 100 // itemHeight:96 + spacing:4
                            radius: Platform.Theme.maxRadius
                            x: (parent.width - width) / 2
                            y: 82
                            alpha: 0
                            opacity: 1
                        }
                        PropertyChanges {
                            target: content
                            contentMargins: 0
                            labelSpacing: 2
                            labelMagrins: 4
                        }
                        PropertyChanges { target: folderNameText; opacity: 1 }
                    },
                    State {
                        name: "folderHidden"
                        PropertyChanges {
                            target: folderIconBase
                            width: 40
                            height: 40
                            radius: Platform.Theme.maxRadius
                            x: root.mapFromGlobal(folderX, 0).x
                            y: root.mapFromGlobal(0, folderY).y
                            alpha: 0.1
                            opacity: 0
                        }
                        PropertyChanges {
                            target: content
                            contentMargins: 3
                            labelSpacing: 0
                            labelMagrins: 0
                        }
                        PropertyChanges { target: folderNameText; opacity: 0 }
                    }
                ]
                transitions: [
                    Transition {
                        to: "folderHidden"
                        SequentialAnimation {
                            ScriptAction { script: content.mouseEnable = false }
                            ParallelAnimation {
                                PropertyAnimation {
                                    target: folderIconBase
                                    duration: animationDuration; easing.type: Easing.InOutCubic
                                    properties: "x, y, width, height, radius, alpha, opacity"
                                }
                                PropertyAnimation {
                                    target: folderNameText
                                    duration: animationDuration; easing.type: Easing.OutQuint
                                    properties: "opacity"
                                }
                                PropertyAnimation {
                                    target: content
                                    duration: animationDuration; easing.type: Easing.OutQuint
                                    properties: "contentMargins, labelMagrins, labelSpacing, labelMagrins"
                                }
                                ScriptAction { script: content.folderSwipeView.normalScreenIconSignal() }
                            }
                            ScriptAction {
                                script: {
                                    // 点击空白区域实现重命名并收起应用组
                                    if (folderNameText.text !== folderNameText.textEdited) {
                                        folderModel.renameFolder(folderNameText.text);
                                    }
                                    content.folderSwipeView.hideFolder()
                                }
                            }
                        }
                    },
                    Transition {
                        to: "folderOpened"
                        SequentialAnimation {
                            ParallelAnimation {
                                PropertyAnimation {
                                    target: folderIconBase
                                    duration: animationDuration; easing.type: Easing.InOutCubic
                                    properties: "x, y, width, height, radius, alpha, opacity"
                                }
                                PropertyAnimation {
                                    target: folderNameText
                                    duration: animationDuration; easing.type: Easing.InQuint
                                    properties: "opacity"
                                }
                                PropertyAnimation {
                                    target: content
                                    duration: animationDuration; easing.type: Easing.OutQuint
                                    properties: "contentMargins, labelMagrins, labelSpacing, labelMagrins"
                                }
                            }
                            ScriptAction {
                                script: {
                                    content.folderSwipeView.contentItem.highlightMoveDuration = 0;
                                    content.mouseEnable = true
                                }
                            }
                        }
                    }
                ]

                FolderContent {
                    id: content
                    anchors.fill: parent
                    folderContentModel: folderModel
                    isFullScreen: false
                }
            }

            PageIndicator {
                id: pageIndicator
                interactive: true
                count: content.count
                visible: count > 1
                currentIndex: content.currentIndex
                anchors.top: folderIconBase.bottom
                anchors.horizontalCenter: folderIconBase.horizontalCenter

                delegate: Rectangle {
                    color: "black"
                    width: 6; height: width;
                    radius: width / 2
                    opacity: (index === content.currentIndex )? 0.9 : 0.3
                }
            }

            AppUI.EditText {
                id: folderNameText
                anchors.bottom: folderIconBase.top
                anchors.bottomMargin: 30
                anchors.horizontalCenter: folderIconBase.horizontalCenter

                height: 47; width: folderIconBase.width
                textEdited: folderName
                textCenterIn: true
                textSize: 16

                onTextEditingFinished: text=> {
                    folderModel.renameFolder(text);
                }
            }
        }
    }
}
