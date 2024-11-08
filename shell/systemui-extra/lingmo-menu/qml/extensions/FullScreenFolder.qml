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

    property int margins: 20
    property int animationDuration: 300
    signal turnPageFinished()

    function initFolder(id, name, x, y) {
        FolderModel.setFolderId(id);
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
                alpha: 0.1

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
                            width: 720 // margins: 16
                            height: viewMaxRow*176 + content.contentMargins*2 // itemHeight: 160 + spacing: 16 + margins: 24
                            radius: Platform.Theme.maxRadius //? 32
                            x: (parent.width - width) / 2
                            y: (parent.height - height) / 2
                        }
                        PropertyChanges {
                            target: content
                            contentMargins: 8
                            labelSpacing: 8
                            labelMagrins: 8
                        }
                        PropertyChanges { target: folderNameText; opacity: 1 }
                    },
                    State {
                        name: "folderHidden"
                        PropertyChanges {
                            target: folderIconBase
                            width: 84
                            height: 84
                            radius: Platform.Theme.maxRadius
                            x: root.mapFromGlobal(folderX, 0).x
                            y: root.mapFromGlobal(0, folderY).y
                        }
                        PropertyChanges {
                            target: content
                            contentMargins: 8
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
                                    duration: animationDuration; easing.type: Easing.InOutCubic
                                    properties: "x, y, width, height, radius, alpha"
                                }
                                PropertyAnimation {
                                    duration: animationDuration; easing.type: Easing.OutQuint
                                    properties: "opacity"
                                }
                                PropertyAnimation {
                                    duration: animationDuration; easing.type: Easing.OutQuint
                                    properties: "contentMargins, labelMagrins, labelSpacing, labelMagrins"
                                }
                            }
                            ScriptAction {
                                script: {
                                    // 点击空白区域实现重命名并收起应用组
                                    if (folderNameText.text !== folderNameText.textEdited) {
                                        folderModel.renameFolder(folderNameText.text);
                                    }
                                    content.folderSwipeView.hideFolder()
                                }}
                        }
                    },
                    Transition {
                        to: "folderOpened"
                        SequentialAnimation {
                            ParallelAnimation {
                                PropertyAnimation {
                                    duration: animationDuration; easing.type: Easing.InOutCubic
                                    properties: "x, y, width, height, radius, alpha"
                                }
                                PropertyAnimation {
                                    duration: animationDuration; easing.type: Easing.InQuint
                                    properties: "opacity"
                                }
                                PropertyAnimation {
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
                    isFullScreen: true
                }
            }

            PageIndicator {
                id: pageIndicator
                interactive: true
                count: content.count
                visible: count > 1
                currentIndex: content.currentIndex
                anchors.bottom: folderIconBase.bottom
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
                anchors.bottomMargin: 12
                anchors.horizontalCenter: folderIconBase.horizontalCenter

                height: 72; width: folderIconBase.width
                textEdited: folderName
                textCenterIn: true
                textSize: 32

                onTextEditingFinished: text=> {
                    folderModel.renameFolder(text);
                }
            }
        }
    }
}
