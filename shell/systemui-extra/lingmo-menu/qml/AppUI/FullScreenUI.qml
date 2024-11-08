import QtQuick 2.0
import QtQuick.Layouts 1.12
import QtQml.Models 2.12

import org.lingmo.menu.core 1.0
import "../extensions" as Extension
import AppControls2 1.0 as AppControls2

import org.lingmo.quick.items 1.0 as LingmoUIItems
import org.lingmo.quick.platform 1.0 as Platform

FocusScope {
    id: fullScreenUI

    Component.onCompleted: {
        forceFocus();
        mainWindow.visibleChanged.connect(forceFocus);
    }
    Component.onDestruction: {
        mainWindow.visibleChanged.disconnect(forceFocus);
    }

    function forceFocus() {
        fullScreenUI.focus = true;
        if (mainWindow.visible) {
            searchInputBar.textInputFocus();
        }
    }

    function keyPressed(event) {
        if ((0x2f < event.key && event.key < 0x3a)||(0x40 < event.key && event.key < 0x5b)) {
            focus = true;
            searchInputBar.text = event.text;
            searchInputBar.textInputFocus();
        }
    }

    Item {
        anchors.fill: parent

        MouseArea {
            anchors.fill: parent
            onClicked: {
                if (mainWindow.editMode) {
                    mainWindow.editMode = false;
                } else {
                    if (mainContainer.visible) {
                        //执行全屏退出操作
                        mainWindow.hide();
                    } else {
                        folderLoader.isFolderOpened = false;
                    }
                }
            }
        }

        Extension.FullScreenFolder {
            id: folderLoader
            anchors.fill: parent

            folderModel: FolderModel
            Component.onCompleted: fullScreenAppList.openFolderSignal.connect(initFolder)
            Component.onDestruction: fullScreenAppList.openFolderSignal.disconnect(initFolder)
        }

        Item {
            id: mainContainer
            anchors.fill: parent
            z: 10

            state: !fullScreenAppList.isContentShow ? "contentShow" : "contentHidden"
            states: [
                State {
                    name: "contentHidden"
                    PropertyChanges { target: mainContainer; opacity: 0; scale: 0.95 }
                },
                State {
                    name: "contentShow"
                    PropertyChanges { target: mainContainer; opacity: 1; scale: 1 }
                }
            ]

            transitions: [
                Transition {
                    to:"contentHidden"
                    SequentialAnimation {
                        PropertyAnimation { properties: "opacity, scale"; duration: 300; easing.type: Easing.InOutCubic }
                        ScriptAction { script: mainContainer.visible = false }
                    }
                },
                Transition {
                    to: "contentShow"
                    SequentialAnimation {
                        ScriptAction { script: mainContainer.visible = true }
                        PropertyAnimation { properties: "opacity, scale"; duration: 300; easing.type: Easing.InOutCubic }
                    }
                }
            ]

            // 两行三列
            GridLayout {
                anchors.fill: parent
                // anchors.margins: 36
                anchors.leftMargin: 36
                anchors.topMargin: 36
                anchors.rightMargin: 36
                anchors.bottomMargin: 5

                rowSpacing: 5
                columnSpacing: 5
                rows: 2
                columns: 3

                // 应用列表模式选择按钮: [row: 0, column: 0] [rowspan: 2, columnSpan: 1]
                Item {
                    Layout.row: 0
                    Layout.column: 0
                    Layout.preferredWidth: Math.max(actionsItem.width, labelsItem.width)
                    Layout.preferredHeight: 40

                    // TODO: 设计最小保持宽度
                    AppListActions {
                        id: actionsItem
                        anchors.centerIn: parent
                        height: parent.height
                        actions: AppPageBackend.appModel.header.actions
                        visible: count > 0
                    }
                }

                // 搜索框: [row: 0, column: 1]
                Item {
                    Layout.row: 0
                    Layout.column: 1
                    Layout.fillWidth: true
                    Layout.preferredHeight: 40

                    SearchInputBar {
                        id: searchInputBar
                        width: 372; height: 36
                        anchors.centerIn: parent
                        radius: Platform.Theme.minRadius
                        visible: opacity

                        onTextChanged: {
                            if (text === "") {
                                AppPageBackend.group = PluginGroup.Display;
                            } else {
                                AppPageBackend.group = PluginGroup.Search;
                                AppPageBackend.startSearch(text);
                            }
                        }

                        Component.onCompleted: {
                            text = root.currentSearchText;
                        }
                    }
                }

                // 右侧按钮区域: [row: 0, column: 2] [rowspan: 2, columnSpan: 1]
                Sidebar {
                    Layout.row: 0
                    Layout.column: 2
                    Layout.rowSpan: 2
                    Layout.columnSpan: 1
                    Layout.preferredWidth: 60
                    Layout.fillHeight: true
                }

                // 左侧标签列表: [row: 1, column: 0]
                Item {
                    Layout.row: 1
                    Layout.column: 0
                    Layout.preferredWidth: Math.max(actionsItem.width, labelsItem.width)
                    Layout.fillHeight: true

                    Item {
                        id: labelsItem
                        anchors.centerIn: parent
                        width: 120
                        height: parent.height
                        clip: true

                        AppLabelPage {
                            id: appLabelPage
                            anchors.centerIn: parent

                            height: (contentHeight > parent.height) ? parent.height : contentHeight
                            width: parent.width
                            labelBottle: AppPageBackend.appModel.labelBottle
                            labelColum: 1
                            cellHeight: 34

                            // TODO: 潜在的优化点，尝试组合widget的model和应用model
                            model: {
                                if (labelBottle !== null) {
                                    let labelItems = [], i = 0, item = null;
                                    let widgetInfos = fullScreenAppList.headerItem.widgetInfos;
                                    for (i = 0; i < widgetInfos.length; ++i) {
                                        item = widgetInfos[i];
                                        labelItems.push({label: item.label, type: item.type, display: item.display});
                                    }

                                    let labels = labelBottle.labels;
                                    for (i = 0; i < labels.length; ++i) {
                                        item = labels[i];
                                        labelItems.push({label: item.label, type: item.type, display: item.display});
                                    }

                                    return labelItems;
                                }

                                return [];
                            }

                            boundsBehavior: Flickable.StopAtBounds
                            interactive: height >= parent.height
                            highlightMoveDuration: 300
                            highlight: LingmoUIItems.StyleBackground {
                                width: appLabelPage.cellWidth
                                height: appLabelPage.cellHeight
                                useStyleTransparency: false
                                radius: Platform.Theme.minRadius
                                paletteRole: Platform.Theme.Light
                                alpha: 0.15; borderAlpha: 0.5
                                border.width: 1
                                borderColor: Platform.Theme.Highlight
                            }

                            onLabelSelected: (label) => {
                                                 fullScreenAppList.positionLabel(label);
                                             }
                        }
                    }
                }

                // 应用列表: [row: 1, column: 1]
                FullScreenAppList {
                    id: fullScreenAppList
                    Layout.row: 1
                    Layout.column: 1
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    sourceModel: AppPageBackend.appModel
                    isContentShow: folderLoader.isFolderOpened

                    function positionLabel(label) {
                        // 如果是Widget那么直接滚动到最顶上
                        // console.log("=positionLabel=", label)
                        if (headerItem.widgets.includes(label)) {
                            positionViewAtBeginning();

                        } else {
                            let index = model.findLabelIndex(label);
                            if (index >= 0) {
                                positionViewAtIndex(index, ListView.Beginning)
                            }
                        }
                    }

                    onContentYChanged: {
                        // 向下偏移200, 计算偏移后在那个item里
                        let index = indexAt(10, contentY + 200);
                        if (index >= 0) {
                            appLabelPage.currentIndex = index + headerItem.widgetCount;
                        } else if (contentY < 0) {
                            appLabelPage.currentIndex = 0;
                        }
                    }

                    Component.onCompleted: {
                        positionViewAtBeginning();
                        folderLoader.turnPageFinished.connect(contentShowFinished)
                    }
                    Component.onDestruction: folderLoader.turnPageFinished.disconnect(contentShowFinished)
                }
            }
        }
    }

}
