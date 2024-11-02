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
 * Authors: qiqi49 <qiqi@kylinos.cn>
 *          hxf <hewenfei@kylinos.cn>
 *
 */

import QtQml 2.15
import QtQuick 2.15
import QtQml.Models 2.15

import org.lingmo.quick.widgets 1.0
import org.lingmo.panel.taskManager 1.0 as LingmoUITaskManager
import org.lingmo.quick.items 1.0 as Items
import org.lingmo.quick.platform 1.0 as Platform

DropArea {
    id: taskItemListRoot

    property var dragedObj: null
    property alias view: taskItemView
    property alias viewModel: taskItemProxyModel
    property bool taskManagerActived: false
    property bool isTaskMenuShow: false
    property bool isDragging: false

    // 请求显示预览图
    signal requestThumbnailView(bool show, TaskItemData data, Item mouseArea)

    LingmoUITaskManager.TaskManagerFilterModel {
        id: taskItemProxyModel
        sourceModel: LingmoUITaskManager.TaskManager
        screen: Widget.container.screen
    }

    Component {
        id: taskIconComponent
        Items.Icon {}
    }

    DelegateModel {
        id: taskItemDelegateModel
        property var dragedObject: null

        /**
         * 删除临时Item
         */
        function removeTempItems() {
            for (let i = tempItems.count - 1; i >= 0; --i) {
                const obj = tempItems.get(i);
                tempItems.remove(i);
                if (obj.inItems) {
                    taskItemDelegateModel.items.remove(obj.itemsIndex);
                }
            }
        }

        /**
         * 将存起来的Item全部归位，并从persistedItems组中移除
         */
        function resetPersistedItems() {
            let i = persistedItems.count - 1;
            for (; i >= 0; --i) {
                const persistedItemObj = persistedItems.get(i);
                if (!persistedItemObj.inItems) {
                    // TODO: listview中的数据往前挪可以同步缓存位置，往后挪则位置不会变，需要把临时Item最后出现的位置记下来
                    let tmpItemsIndex = persistedItemObj.itemsIndex;
                    persistedItems.addGroups(i, 1, [items.name]);
                    items.move(persistedItemObj.itemsIndex, tmpItemsIndex, 0);
                }
                persistedItems.remove(i, 1);
            }

            dragedObject = null;
            removeTempItems();
        }

        // 临时生成的用于定位拖拽落点的Items组
        groups: [
            DelegateModelGroup { id: tempItems; name: "tempItems" }
        ]

        model: taskItemProxyModel
        delegate: Item {
            id: taskItemRoot

            property int dragStartIndex: -1
            property var icon: model.Icon
            property bool isDragStatus: false
            property bool useSingleItem: taskItemView.isMergeStatus || (CurrentWinIdList ? CurrentWinIdList.length < 1 : true)
            //非合并状态下激活预览图时 对应的winId
            property var singleWinId: []
            property bool iconNeedsUpdate: true

            width: taskItemView.isHorizontal ? taskItemLoader.width : taskItemView.width
            height: taskItemView.isHorizontal ? taskItemView.height : taskItemLoader.height
            visible: DelegateModel.inItems || !isDragStatus

            onIconChanged: {
                iconNeedsUpdate = true;
            }

            function generateDragImage() {
                let icon = taskIconComponent.createObject(this, {width: taskItemView.taskItemSize, height: taskItemView.taskItemSize, source: model.Icon});
                if (icon !== null) {
                    icon.grabToImage(function (result) {
                        taskItemRoot.Drag.imageSource = result.url;
                        icon.destroy();
                    });
                    icon.visible = false
                }
            }

            // 拖拽设置, 手动开启拖拽
            Drag.dragType: Drag.None
            Drag.hotSpot: Qt.point(taskItemView.taskItemSize/2, taskItemView.taskItemSize/2)
            // 处理拖拽完成信号
            Drag.onDragFinished: function (dropAction) {
                taskItemListRoot.isDragging = false;
                // 拖拽被取消，仅处理内部拖拽
                if (DelegateModel.inPersistedItems) {
                    const myPObj = taskItemDelegateModel.persistedItems.get(DelegateModel.persistedItemsIndex);
                    taskItemDelegateModel.persistedItems.addGroups(myPObj.persistedItemsIndex, 1, [taskItemDelegateModel.items.name]);
                    // 回到启动拖拽的位置
                    const idx = (dragStartIndex < 0) ? myPObj.itemsIndex : (dragStartIndex >= taskItemDelegateModel.items.count) ? taskItemDelegateModel.items.count : dragStartIndex
                    if (myPObj.itemsIndex !== idx) {
                        taskItemDelegateModel.items.move(myPObj.itemsIndex, idx, 1);
                    }
                }

                taskItemDelegateModel.resetPersistedItems();
                taskItemRoot.isDragStatus = false;
            }

            function startDrag() {
                // 存起来的被拖拽项目和临时生成的项目都不允许被拖动
                if (!DelegateModel.inItems || DelegateModel.inPersistedItems || DelegateModel.inTempItems) {
                    return;
                }

                const delegateModel = DelegateModel.model;
                const itemsIndex = DelegateModel.itemsIndex;
                dragStartIndex = itemsIndex;
                // 当前拖拽对象
                delegateModel.dragedObject = delegateModel.items.get(itemsIndex);
                // 将被拖拽的项添加到持久化组中
                delegateModel.items.addGroups(itemsIndex, 1, [delegateModel.persistedItems.name]);
                // 从界面移除当前被拖动的项目
                let draggedObj = taskItemDelegateModel.persistedItems.get(0);
                // 如果在items组中，那么从可见列表移除
                if (draggedObj.inItems) {
                    taskItemDelegateModel.items.removeGroups(draggedObj.itemsIndex, 1, ["items"]);
                }

                taskItemRoot.isDragStatus = true;
                const modelIndex = delegateModel.dragedObject.model.index;
                Drag.mimeData = {
                    "text/plain": '{"dragStartIndex":' + dragStartIndex + ',"modelIndex":' +  modelIndex + '}'
                };
                Drag.active = true;
                // 不同的Action鼠标光标不同
                Drag.startDrag(Qt.MoveAction);
            }

            Binding {
                target: taskItemListRoot
                property: "taskManagerActived"
                when: model.DemandsAttentionWinIdList ? (model.DemandsAttentionWinIdList.length !== 0) : false
                value: true
            }

            MouseArea {
                id: mouseArea
                x: 0; y: 0
                width: parent.width; height: parent.height
                hoverEnabled: true
                acceptedButtons: Qt.LeftButton | Qt.RightButton
                // enabled: !taskItemRoot.DelegateModel.inTempItems

                property int lastIndex: -1
                property string currentWid: ""

                function activeWinId(winId) {
                    // 隐藏预览图
                    taskItemListRoot.requestThumbnailView(false, null, mouseArea);
                    if (LingmoUITaskManager.TaskManager.windowIsActivated(winId)) {
                        LingmoUITaskManager.TaskManager.execSpecifiedAction(LingmoUITaskManager.Action.Minimize, winId);
                    } else {
                        LingmoUITaskManager.TaskManager.activateWindow(winId);
                    }
                }

                // 为拖拽截图
                onPressed:  {
                    if (iconNeedsUpdate) {
                        parent.generateDragImage();
                        iconNeedsUpdate = false;
                    }
                }
                drag.target: mouseArea
                drag.onActiveChanged: {
                    taskItemListRoot.requestThumbnailView(false, null, mouseArea);
                    if (drag.active) {
                        taskItemListRoot.isDragging = true;
                        taskItemRoot.startDrag();
                    } else {
                        mouseArea.x = 0;
                        mouseArea.y = 0;
                    }
                }

                onClicked: (mouse) => {
                    if (mouse.button === Qt.RightButton) {
                        taskItemProxyModel.openMenu(false, model.Actions, taskItemView);
                        taskManagerActived = true;
                        isTaskMenuShow = true;
                        taskItemListRoot.requestThumbnailView(false, null, mouseArea);

                    } else {
                        if (CurrentWinIdList.length === 0) {
                            taskItemDelegateModel.model.launch(taskItemDelegateModel.model.index(model.index, 0));
                            return;
                        }

                        let index = -1;
                        if (taskItemView.isMergeStatus) {
                            // 轮转
                            if (currentWid !== "") {
                                index = CurrentWinIdList.indexOf(currentWid);
                                if (index < (CurrentWinIdList.length - 1)) {
                                    ++index;
                                } else {
                                    index = 0;
                                }
                            }
                        } else {
                            // 由taskList实现
                            index = taskItemLoader.item.indexAt(mouseX, mouseY);
                        }

                        if (index < 0) {
                            index = 0;
                        } else if (index >= CurrentWinIdList.length) {
                            index = CurrentWinIdList.length - 1;
                        }

                        currentWid = CurrentWinIdList[index];
                        activeWinId(currentWid);
                    }
                }

                onEntered: {
                    // 设置预览图窗口信息
                    if (CurrentWinIdList !== undefined && CurrentWinIdList.length > 0) {
                        if (taskItemView.isMergeStatus) {
                            taskItemData.currentTaskItem = taskItemRoot;
                            taskItemListRoot.requestThumbnailView(true, taskItemData, mouseArea);
                        }
                    }
                }

                onExited: {
                    taskItemListRoot.requestThumbnailView(false, taskItemData, mouseArea);
                }

                TaskItemData {
                    id: taskItemData
                    windowIdList: model.CurrentWinIdList
                    windowIcons:  model.WindowIcons
                    windowTitles: model.WindowTitles
                }

                Items.Tooltip {
                    id: tooltip
                    anchors.fill: parent
                    mainText: Name
                    posFollowCursor: false
                    // 只有固定在任务栏的应用显示tooltip
                    active: CurrentWinIdList ? CurrentWinIdList.length < 1 : false
                    location: {
                        switch(Widget.container.position) {
                            case Items.Types.Bottom:
                                return Items.Dialog.BottomEdge;
                            case Items.Types.Left:
                                return Items.Dialog.LeftEdge;
                            case Items.Types.Top:
                                return Items.Dialog.TopEdge;
                            case Items.Types.Right:
                                return Items.Dialog.RightEdge;
                        }
                    }
                    margin: 6
                }

                Loader {
                    id: taskItemLoader
                    property var taskItemData: model

                    // 在非合并状态下，由loader里的item触发request信号和参数
                    function loadedItemRequest(show, data, item) {
                        data.currentTaskItem = item;
                        taskItemListRoot.requestThumbnailView(show, data, item);
                    }

                    sourceComponent: {
                        if (taskItemRoot.DelegateModel.inTempItems) {
                            return tempTaskitem;
                        }

                        if (taskItemRoot.useSingleItem) {
                            return singleTaskItem;
                        }

                        return listTaskItem;
                    }

                    onLoaded: {
                        if (!taskItemRoot.useSingleItem) {
                            item.onRequestThumbnailView.connect(loadedItemRequest);
                        }
                    }

                    Component.onDestruction: {
                        if (!taskItemRoot.useSingleItem) {
                            item.onRequestThumbnailView.disconnect(loadedItemRequest);
                        }
                    }
                }

                Component {
                    id: tempTaskitem
                    Item {
                        width: taskItemView.taskItemSize
                        height: taskItemView.taskItemSize
                    }
                }

                // 非合并状态 图标+名称
                Component {
                    id: listTaskItem
                    AppList {
                        height: childrenRect.height
                        width: childrenRect.width

                        itemSize: taskItemView.taskItemSize
                        isDragged: taskItemRoot.isDragStatus
                        orientation: taskItemView.orientation
                        spacing: taskItemView.spacing
                        model: taskItemData.CurrentWinIdList
                    }
                }

                Component {
                    id: singleTaskItem
                    AppIcon {
                        width: taskItemView.taskItemSize
                        height: taskItemView.taskItemSize
                        appBackgroud.alpha: mouseArea.containsPress ? 0.10 : mouseArea.containsMouse ? 0.05 : 0
                        appIcon.source: taskItemData.Icon
                        appIcon.scale: mouseArea.containsPress ? 0.9 : 1.0
                    }
                }
            }

            /**
             * iconGeometry功能
             */
            property var currentWinIdList: model.CurrentWinIdList
            property int itemIndex: DelegateModel.itemsIndex
            property bool invisibleOfFront: (x < taskItemView.contentX) || (y < taskItemView.contentY)
            property bool invisibleOfBack: (x > taskItemView.width + taskItemView.contentX) || (y > taskItemView.height + taskItemView.contentY)

            function updateIconGeometry() {
                if (taskItemLoader.item && !taskItemRoot.DelegateModel.inTempItems) {
                    taskItemLoader.item.updateIconGeometry();
                }
            }

            function addIndexToGeometryList() {
                if (model.CurrentWinIdList) {
                    if (model.CurrentWinIdList.length > 0 && !taskItemView.pendingUpdatesList.includes(itemIndex)) {
                        taskItemView.pendingUpdatesList.push(itemIndex);
                        updateGeometryList.restart();
                    }
                }
            }
            Component.onCompleted: {
                //初始化、拖拽后更新model、切换任务栏方向，都会触发
                addIndexToGeometryList();
            }
            onCurrentWinIdListChanged: {
                addIndexToGeometryList();
            }
        }
    }

    ListView {
        id: taskItemView
        anchors.fill: parent
        // contentWidth: childrenRect.width
        // width: childrenRect.width
        // height: parent.height

        property bool windowVisible: false
        property bool isMergeStatus: {
            var status = Widget.globalConfig.mergeIcons;
            return (status === undefined) || (status === 0) ;
        }
        property bool isHorizontal: orientation === ListView.Horizontal
        property real taskItemSize: isHorizontal ? taskItemView.height : taskItemView.width
        clip: true
        interactive: false

        spacing: 4
        orientation: (Widget.orientation === Items.Types.Horizontal) ? ListView.Horizontal : ListView.Vertical
        model: taskItemDelegateModel
        onTaskItemSizeChanged: {
            pushAllIndex();
        }
        onIsMergeStatusChanged: {
            pushAllIndex();
        }
        onWindowVisibleChanged: {
            if(Platform.WindowManager.isWaylandSession) {
                pushAllIndex();
            }
        }
        onContentXChanged: {
            pushAllIndex();
        }
        onContentYChanged: {
            pushAllIndex();
        }
        function updateIconState() {
            for (let i = 0; i < taskItemDelegateModel.count; ++i) {
                taskItemView.itemAtIndex(i).iconNeedsUpdate = true;
            }
        }

        Component.onCompleted: {
            Platform.Theme.iconThemeChanged.connect(updateIconState);
            if (Widget.globalConfig.mergeIcons === undefined) {
                Widget.globalConfig.mergeIcons = 0;
            }
        }

        /**
         * iconGeometry
         */
        property var pendingUpdatesList: []

        Connections {
            target: Widget.container
            function onPositionChanged() {
                taskItemView.pushAllIndex();
            }
        }

        function pushAllIndex() {
            pendingUpdatesList.length = 0;
            for (var i = 0; i < taskItemView.count; ++i) {
                pendingUpdatesList.push(i);
            }
            if (pendingUpdatesList.length > 0) {
                updateGeometryList.restart();
            }
        }

        Timer {
            id: updateGeometryList
            interval: 1000

            onTriggered: {
                for (let i = 0; i < taskItemView.pendingUpdatesList.length; ++i) {
                    let index = taskItemView.pendingUpdatesList[i];
                    let item = taskItemView.itemAtIndex(index);
                    if (item) {
                        if (item.invisibleOfFront) {
                            Platform.WindowManager.setMinimizedGeometry(item.currentWinIdList, virtualFrontItem);
                        } else if (item.invisibleOfBack) {
                            Platform.WindowManager.setMinimizedGeometry(item.currentWinIdList, virtualBackItem);
                        } else {
                            item.updateIconGeometry();
                        }
                    }
                }
                taskItemView.pendingUpdatesList.length = 0;
            }
        }

        Item {
            id: virtualFrontItem
            x: 0; y: 0
            width: 10
            height: 10
        }
        Item {
            id: virtualBackItem
            x: taskItemView.width - width; y: taskItemView.height - height
            width: 10
            height: 10
        }

        displaced: Transition {
            NumberAnimation {
                properties: "x,y"
                easing.type: Easing.OutQuad
                duration: 300
            }
        }

        move: Transition {
            NumberAnimation { properties: "x,y"; duration: 300 }
        }

        add: Transition {
            NumberAnimation {
                id: addAnimation
                property: taskItemView.isHorizontal ? "y" : "x"
                from: (Widget.container.position === Items.Types.Left) || (Widget.container.position === Items.Types.Top) ? -taskItemView.taskItemSize : taskItemView.taskItemSize
                to: 0
                easing.type: Easing.OutCubic
                duration: 300
            }
        }

        remove: Transition {
            NumberAnimation {
                id: removeAnimation
                property: addAnimation.property
                easing.type: Easing.OutCubic
                from: 0; to: addAnimation.from
                duration: 300
            }
        }
    }

    // 获取ListView的坐标对应的ContentItem内部坐标
    // listview与dropArea的大小需要重合
    function getContentPoint(localX, localY) {
        return Qt.point(localX + taskItemView.contentX, localY + taskItemView.contentY);
    }

    /**
     * 事件来源判断
     * 内部: true
     * 外部: false
     */
    function isInternalEvent() {
        return taskItemDelegateModel.dragedObject !== null;
    }

    // 获取一个Item的中点坐标
    function getItemMidPoint(item) {
        return Qt.point(item.x + item.width/2, item.y + item.height/2);
    }

    // point 是否在item的中点前
    function beforeMidpoint(item, point) {
        let midPoint = getItemMidPoint(item);
        if (taskItemView.orientation === ListView.Horizontal) {
            return point.x < midPoint.x;
        } else {
            return point.y < midPoint.y
        }
    }

    // 1.拖拽进入事件
    // 拖拽进入，需要生成临时Item
    onEntered: (dragEvent) => {
        // 1.处理进入拖拽区域时的边缘点情况，避免丢失拖拽事件
        let enterX = dragEvent.x === width ? dragEvent.x - 1 : dragEvent.x;
        let enterY = dragEvent.y === height ? dragEvent.y - 1 : dragEvent.y;
        const contentPoint = getContentPoint(enterX, enterY);

        // 2.删除上次未删除的临时Item,保证只有一个临时Item
        if (tempItems.count > 0) {
            taskItemDelegateModel.removeTempItems();
        }

        // 3.获取进入点下方的Item, 如果为null，则说明拖拽到了空白位置
        let insertIndex = -1;
        let pointItem = taskItemView.itemAt(contentPoint.x, contentPoint.y);
        if (pointItem !== null) {
            // 4.通过进入点的对象的itemsIndex获取各种数据。
            insertIndex = pointItem.DelegateModel.itemsIndex;

            // 判断中点位置，确定在插入在前还是后，insertIndex or insertIndex+1
            const maxIdx = taskItemDelegateModel.items.count - 1;
            if ((insertIndex > 0) && (insertIndex < maxIdx) && beforeMidpoint(pointItem, contentPoint)) {
                ++insertIndex;
            }
        } else {
            // 5.拖拽进入点是空白区域
            insertIndex = taskItemDelegateModel.items.count;
        }

        /**
         * 4.判断事件来源
         * 区分拖拽事件的来源，一般有两个来源：
         * 1.内部拖拽事件，拖拽listview的Item进行位置的调整。
         * 2.外部事件，将桌面图标拖拽到任务栏上。
         */
        const internal = isInternalEvent();
        if (!internal) {
            // 暂不支持外部拖拽
            dragEvent.accepted = false;
            return;
        }

        // 5.创建临时Item，并将他放到tempItems组中，后续可通过该组删除它
        if (insertIndex < 0) {
            insertIndex = 0;
        } else if (insertIndex > taskItemDelegateModel.items.count) {
            insertIndex = taskItemDelegateModel.items.count;
        }

        taskItemDelegateModel.items.insert(
            insertIndex,
            {
                Name: ""
            },
            [taskItemDelegateModel.items.name, "tempItems"]
        );
    }

    // 2.拖拽移出事件
    onExited: {
        taskItemDelegateModel.removeTempItems();
    }

    // 3.拖拽移动事件
    onPositionChanged: (dragEvent) => {
        if (tempItems.count === 1) {
            // 落点
            let currentPoint = getContentPoint(dragEvent.x, dragEvent.y);
            let targetItem = taskItemView.itemAt(currentPoint.x, currentPoint.y);
            if (targetItem !== null && !targetItem.DelegateModel.inTempItems) {
                let sourceIndex = tempItems.get(0).itemsIndex;
                let targetIndex = targetItem.DelegateModel.itemsIndex;

                const bmp = beforeMidpoint(targetItem, currentPoint);
                if ((sourceIndex < targetIndex) && bmp) {
                    --targetIndex;
                } else if ((sourceIndex > targetIndex) && !bmp) {
                    ++targetIndex;
                }

                let moveCount = Math.abs(targetIndex - sourceIndex);
                if (moveCount === 0) {
                    return;
                }

                if (sourceIndex < targetIndex) {
                    taskItemDelegateModel.items.move(sourceIndex + 1, sourceIndex, moveCount);
                } else {
                    taskItemDelegateModel.items.move(targetIndex, targetIndex + 1, moveCount);
                }
            }
        }
    }

    // 4.拖拽放下事件
    onDropped: (dragEvent) => {
        // 1.获取丢下点的Index
        let droppedIndex = -1;
        if (tempItems.count > 0) {
            // 默认情况下临时Item对应的位置就是丢下的位置
            let tempObj = tempItems.get(0);
            droppedIndex = tempObj.inItems ? tempObj.itemsIndex : taskItemDelegateModel.items.count;
        } else {
            // 修正边缘落点情况
            let enterX = dragEvent.x === width ? dragEvent.x - 1 : dragEvent.x;
            let enterY = dragEvent.y === height ? dragEvent.y - 1 : dragEvent.y;
            let contentPoint = getContentPoint(enterX, enterY);

            let dropTargetItem = taskItemView.itemAt(contentPoint.x, contentPoint.y);
            droppedIndex = (dropTargetItem === null) ? taskItemDelegateModel.items.count : dropTargetItem.DelegateModel.itemsIndex;
        }

        // 2.清空临时对象
        taskItemDelegateModel.removeTempItems();

        // 3.事件类型
        if (isInternalEvent()) {
            let persistedObj = taskItemDelegateModel.dragedObject;
            // 将存起来的Item放到items组中
            taskItemDelegateModel.persistedItems.addGroups(0, 1, ["items"])
            // 挪到正确位置
            if (persistedObj.itemsIndex !== droppedIndex) {
                taskItemDelegateModel.items.move(persistedObj.itemsIndex, droppedIndex, 1);
            }
            // 清空保存的Item
            taskItemDelegateModel.resetPersistedItems();

            const obj = JSON.parse(dragEvent.text);
            if (obj !== null) {
                taskItemProxyModel.setOrder(taskItemDelegateModel.model.index(obj.modelIndex, 0), droppedIndex);
            }
        } else {
            // 外部事件，执行外部操作
        }
        taskItemListRoot.isDragging = false;
    }
}
