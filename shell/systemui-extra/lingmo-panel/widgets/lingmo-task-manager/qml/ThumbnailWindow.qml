/*
 *
 *  Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 *  Authors: qiqi49 <qiqi@kylinos.cn>
 *
 */

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15

import org.lingmo.quick.items 1.0
import org.lingmo.windowThumbnail 1.0
import org.lingmo.quick.platform 1.0 as Platform
import org.lingmo.panel.taskManager 1.0 as LingmoUITaskManager

MouseArea {
    id: baseMouseArea

    property bool isList
    property var thumbnailLayout: viewLoader.item.viewRepeater
    hoverEnabled: true

    property bool menuVisible: false
    property int layoutSpacing: 8
    property int thumbnailImplicitHeight: 192
    // 设置初始高度，规避初次显示时出现鼠标悬浮退出事件，影响预览图显示
    width: 272
    height: 192

    function determineListViewSize() {
        if (!currentWinIdList) {
            return;
        }
        if (isHorizontal) {
            var listPreHeight = listViewBase.determineHeight(currentWinIdList).height;
            if (listPreHeight < 124) {
                baseMouseArea.isList = true;
                return;
            } else {
                viewLoader.preChildrenSize = listPreHeight;
                baseMouseArea.isList = false;
            }
        } else {
            var listPreWidth = listViewBase.determineWidth(currentWinIdList).width;
            if (listPreWidth < 124) {
                baseMouseArea.isList = true;
                return;
            } else {
                viewLoader.preChildrenSize = listPreWidth;
                baseMouseArea.isList = false;
            }
        }
    }

    StyleBackground {
        id: listViewBase
        paletteRole: Platform.Theme.Base
        useStyleTransparency: false

        width: viewLoader.width
        height: viewLoader.height
        alpha: isList ? 0.75 : 0
        borderColor: Platform.Theme.Text
        borderAlpha: 0.15
        border.width: isList ? 1 : 0
        radius: Platform.Theme.windowRadius

        // 窗口尺寸不能为0
        onWidthChanged: {
            baseMouseArea.width = Math.max(Math.ceil(listViewBase.width), 1);
            baseMouseArea.height = Math.max(Math.ceil(listViewBase.height), 1);
        }
        onHeightChanged: {
            baseMouseArea.width = Math.max(Math.ceil(listViewBase.width), 1);
            baseMouseArea.height = Math.max(Math.ceil(listViewBase.height), 1);
        }

        function determineHeight(winIdList) {
            var totalWidth = 0;
            var thumbnailHeight = 144;
            if (!winIdList) {
                return;
            }

            for (var i = 0; i < winIdList.length; i++) {
                var itemGeometry = Platform.WindowManager.geometry(winIdList[i]);
                totalWidth = totalWidth + (itemGeometry.width * (thumbnailHeight / itemGeometry.height));
            }

            var otherAreaSize;
            // 其余区域的大小，包括间距和预览图的margin
            otherAreaSize = layoutSpacing * 2 * winIdList.length + 6 * (winIdList.length - 1);
            if ((Screen.width - 16 - otherAreaSize) < totalWidth) {
                thumbnailHeight = thumbnailHeight * (Screen.width - 16 - otherAreaSize) / totalWidth;
            }

            var listPreHeight = thumbnailHeight + 48;
            var listPreWidth = otherAreaSize + totalWidth;
            return Qt.rect(0, 0, listPreWidth, listPreHeight);
        }

        function determineWidth(winIdList) {
            var totalHeight = 0;
            var thumbnailWidth = 176;
            if (!winIdList) {
                return;
            }

            for (var i = 0; i < winIdList.length; i++) {
                var itemGeometry = Platform.WindowManager.geometry(winIdList[i]);
                totalHeight = totalHeight + (itemGeometry.height * (thumbnailWidth / itemGeometry.width));
            }

            var otherAreaSize;
            // 其余区域的大小，包括间距和预览图的margin,以及纵向布局时的title高度（24）
            otherAreaSize = (layoutSpacing * 3 + 24) * winIdList.length + 6 * (winIdList.length - 1);

            if ((Screen.height - 16 - otherAreaSize) < totalHeight) {
                thumbnailWidth = thumbnailWidth * (Screen.height - 16 - otherAreaSize) / totalHeight;
            }

            var listPreWidth = thumbnailWidth + 16;
            var listPreHeight = totalHeight + otherAreaSize;
            return Qt.rect(0, 0, listPreWidth, listPreHeight);
        }

        Loader {
            id: viewLoader
            property int preChildrenSize
            sourceComponent: baseMouseArea.isList ? listMode : thumbnailMode
        }

        /**
         * 预览图显示View： 列表模式、预览图模式
         */
        Component {
            id: thumbnailMode
            Item {
                width: childrenRect.width
                height: childrenRect.height
                property var viewRepeater: repeater

                Column {
                    id: columnLayout
                    spacing: 6
                }
                Row {
                    id: rowLayout
                    spacing: 6
                }
                Repeater {
                    id: repeater
                    parent: isHorizontal ? rowLayout : columnLayout
                    model: thumbnailModel
                    delegate: thumbnailDelegate
                }
            }
        }
    }
    Component {
        id: listMode
        Row {
            padding: 4
            spacing: 0

            ListView {
                width: 220
                height: 360
                interactive: true
                spacing: 0
                orientation: ListView.Vertical
                ScrollBar.vertical: scrollBarControl
                boundsBehavior: Flickable.StopAtBounds
                model: thumbnailModel
                delegate: thumbnailDelegate
            }
            ScrollBar {
                id: scrollBarControl
                height: 360
                width: (size < 1 && size > 0) ? 12 : 0
                padding: 4
                contentItem: StyleBackground {
                    radius: width / 2
                    useStyleTransparency: false
                    paletteRole: Platform.Theme.Text
                    alpha: 0.3
                }
            }
        }
    }
    /**
     * 预览图delegate
     */
    Component {
        id: thumbnailDelegate
        MouseArea {
            id: itemMouseArea
            width: childrenRect.width
            height: childrenRect.height
            hoverEnabled: true
            acceptedButtons: Qt.LeftButton | Qt.RightButton
            property alias itemRegion: blurRegion

            RectRegion {
                id: blurRegion
                x: itemMouseArea.x
                y: itemMouseArea.y
                width: itemMouseArea.width
                height: itemMouseArea.height
                radius: thumbnailLoader.item.radius
            }

            Timer {
                id: activeTimer
                property var activeWinIds: []
                onTriggered: {
                    LingmoUITaskManager.TaskManager.activateWindowView(activeTimer.activeWinIds)
                }
            }

            function activateWindowView() {
                var winIds = [model.winId];
                if (baseMouseArea.menuVisible || containsMouse) {
                    activeTimer.interval = 500;
                    activeTimer.activeWinIds = winIds;
                    activeTimer.start();
                } else {
                    activeTimer.stop();
                    LingmoUITaskManager.TaskManager.activateWindowView([]);
                }
            }

            onContainsMouseChanged: activateWindowView()

            onClicked: {
                if (mouse.button === Qt.RightButton) {
                    var actions = LingmoUITaskManager.TaskManager.windowActions(model.winId);
                    baseMouseArea.menuVisible = taskManager.viewModel.openMenu(true, actions, viewLoader);
                } else {
                    LingmoUITaskManager.TaskManager.activateWindow(model.winId);
                }
            }
            Loader {
                id: thumbnailLoader
                WindowThumbnailMprisModel {
                    id: mprisModel
                    winID: model.winId
                    pid: Platform.WindowManager.pid(model.winId)
                }
                property string modelData: model.winId
                property var title: windowTitles ? windowTitles[model.winId] : ""
                property var icon: windowIcons ? windowIcons[model.winId] : ""
                property var windowThumbnailMprisModel: mprisModel
                property bool mouseAreaContainsMouse: itemMouseArea.containsMouse
                property bool mouseAreaContainsPress: itemMouseArea.containsPress
                property bool isListViewHorizontal: isHorizontal

                // 预览图限制宽高
                property int maximumHeight: 312
                property int minimumHeight: 120
                property int maximumWidth: 502
                property int minimumWidth : 120

                // mprisModel.count > 0为mpris音视频预览图
                sourceComponent: {
                    if (mprisModel.count > 0) {
                        if (mprisModel.isCurrentMediaAudio) {
                            return musicThumbanilComponent;
                        } else {
                            return videoThumbnailComponent;
                        }
                    } else {
                        return thumbnailComponent;
                    }
                }
            }
        }
    }
    /**
     * 预览图组件显示: 普通预览图（预览图界面、列表界面）、mpris音频预览图、mpris视频预览图
     */
    Component {
        id: thumbnailComponent
        ThumbnailContainer {
            width: baseMouseArea.isList ? 220 : (isListViewHorizontal ? contentWidth : viewLoader.preChildrenSize)
            height: baseMouseArea.isList ? 36 : (isListViewHorizontal ? viewLoader.preChildrenSize : contentHeight)

            property real contentHeight: childrenRect.height
            property real contentWidth: childrenRect.width

            isList: baseMouseArea.isList
            spacing: layoutSpacing
            containerIcon: icon
            containerTitle: title === undefined ? "" : title
            containsMouse: mouseAreaContainsMouse
            containsPress: mouseAreaContainsPress
            viewIsHorizontal: isListViewHorizontal

            onCloseButtonClicked: {
                LingmoUITaskManager.TaskManager.activateWindowView([]);
                LingmoUITaskManager.TaskManager.execSpecifiedAction(LingmoUITaskManager.Action.Close, modelData);
            }
        }
    }
    Component {
        id: musicThumbanilComponent
        StyleBackground {
            paletteRole: Platform.Theme.Base
            radius: Platform.Theme.windowRadius
            useStyleTransparency: false
            alpha: 0.75
            width: childrenRect.width
            height: childrenRect.height
            borderColor: Platform.Theme.Text
            borderAlpha: 0.15
            border.width: 1


            AudioPalyerThumbnail {
                playerData: windowThumbnailMprisModel
                containsMouse: mouseAreaContainsMouse
                windowRadius: parent.radius
                onCloseButtonClicked: {
                    LingmoUITaskManager.TaskManager.activateWindowView([]);
                    LingmoUITaskManager.TaskManager.execSpecifiedAction(LingmoUITaskManager.Action.Close, modelData);
                }
            }
        }
    }
    Component {
        id: videoThumbnailComponent
        StyleBackground {
            paletteRole: Platform.Theme.Base
            radius: Platform.Theme.windowRadius
            useStyleTransparency: false
            alpha: 0.75
            width: childrenRect.width
            height: childrenRect.height
            property int thumbnailMargin: 8
            borderColor: Platform.Theme.Text
            borderAlpha: 0.15
            border.width: 1


            Repeater {
                id: view
                model: windowThumbnailMprisModel
                ColumnLayout {
                    id: baseLayout
                    spacing: 0
                    height: {
                        var preHeight = windowThumbnail.height + thumbnailMargin * 3 + 24;
                        if (preHeight < minimumHeight ) {
                            return minimumHeight;
                        } else if (preHeight > maximumHeight) {
                            windowThumbnail.Layout.maximumHeight = maximumHeight - thumbnailMargin * 3 - 24;
                            return maximumHeight
                        } else {
                            return preHeight;
                        }
                    }
                    width: {
                        var preWidth = windowThumbnail.width + thumbnailMargin * 2;
                        if (preWidth < minimumWidth ) {
                            return minimumWidth;
                        } else if (preWidth > maximumWidth) {
                            windowThumbnail.Layout.maximumWidth = maximumWidth - thumbnailMargin * 2;
                            return maximumWidth
                        } else {
                            return preWidth;
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 24
                        Layout.maximumHeight: 24
                        Layout.margins: thumbnailMargin
                        Layout.bottomMargin: 0

                        ThemeIcon {
                            Layout.preferredHeight: 24
                            Layout.preferredWidth: 24
                            Layout.rightMargin: thumbnailMargin
                            source: icon
                        }
                        StyleText {
                            Layout.fillHeight: true
                            Layout.fillWidth: true
                            text: title === undefined ? "" : title
                            elide: Text.ElideRight
                            verticalAlignment: Text.AlignVCenter
                        }
                        MouseArea {
                            Layout.preferredHeight: 16
                            Layout.preferredWidth: 16
                            Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                            hoverEnabled: true
                            visible: mouseAreaContainsMouse

                            Rectangle {
                                color: "red"
                                radius: 4
                                anchors.fill: parent
                                visible: parent.containsMouse
                            }
                            Icon {
                                anchors.fill: parent
                                mode: parent.containsMouse ? Icon.Highlight : Icon.AutoHighlight
                                source: "window-close-symbolic"
                            }
                            onClicked: {
                                LingmoUITaskManager.TaskManager.activateWindowView([]);
                                LingmoUITaskManager.TaskManager.execSpecifiedAction(LingmoUITaskManager.Action.Close, modelData);
                            }
                        }
                    }
                    VideoPlayerThumbnail {
                        id: windowThumbnail
                        Layout.alignment: Qt.AlignHCenter
                        Layout.margins: thumbnailMargin
                        Layout.preferredWidth: width
                        Layout.preferredHeight: height

                        height: isListViewHorizontal ? viewLoader.preChildrenSize - 48 : itemHeight
                        width: isListViewHorizontal ? itemWidth : viewLoader.preChildrenSize - 16
                        property real itemHeight
                        property real itemWidth
                        winId: windowVisible ? modelData : ""
                        playerData: windowThumbnailMprisModel
                        viewModel: view.model
                        onWinIdChanged: {
                            if (winId === "") {
                                return;
                            }
                            var itemGeometry = Platform.WindowManager.geometry(modelData);
                            if (isListViewHorizontal) {
                                itemWidth = itemGeometry.width * ((viewLoader.preChildrenSize - 48) / itemGeometry.height);
                            } else {
                                itemHeight = itemGeometry.height * ((viewLoader.preChildrenSize - 16) / itemGeometry.width);
                            }
                        }
                    }
                }
            }
        }
    }
}
