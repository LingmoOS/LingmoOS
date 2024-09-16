// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts
import QtQml.Models
import QtQml
import QtQuick.Shapes 1.10
import org.deepin.dtk 1.0
import org.deepin.album 1.0 as Album

import "../"
import "../../"
import "../../PreviewImageViewer"
import "../../PreviewImageViewer/Utils"

import "ThumbnailListViewTools.js" as ThumbnailTools

FocusScope {
    id : main

    property alias proxyModel: thumbnailModel
    property QtObject model: thumbnailModel

    property Item rubberBand: null

    property alias enableListenrMouse: listener.enableMouse //使能鼠标操作
    property bool enableMouse: true

    // 缩略图Item尺寸
    property real itemWidth: realCellWidth
    property real itemHeight: realCellWidth
    // 缩略图显示类型，默认为普通模式
    property int thumnailListType: Album.Types.ThumbnailNormal
    // 存在已选项
    property bool haveSelect: thumbnailModel.selectedIndexes.length > 0
    // 已选择全部缩略图
    property bool haveSelectAll: thumbnailModel.selectedIndexes.length === thumbnailModel.rowCount() && thumbnailModel.selectedIndexes.length > 0

    // 已选项个数
    property int haveSelectCount: thumbnailModel.selectedIndexes.length
    // 已选路径
    property var selectedUrls: thumbnailModel.selectedUrls
    // 已选原始路径
    property var selectedPaths: thumbnailModel.selectedPaths
    // 所有项个数
    property var count: thumbnailModel.count

    //缩略图动态变化
    property real realCellWidth:  {
        var rowSizeHint = parseInt((width - GStatus.thumbnailListRightMargin) / GStatus.cellBaseWidth)
        return (width - GStatus.thumbnailListRightMargin) / rowSizeHint
    }

    //当前区域时间区间改变信号，字符串可以直接刷新在界面上
    signal timeChanged(string str)
    signal selectedChanged()

    //设置全选/全取消缩略图
    function selectAll(doSelect) {
        if (gridView.visible) {
            if(doSelect) {
                thumbnailModel.selectAll()
            } else {
                thumbnailModel.clearSelection()
            }
        }
        selectedChanged()
    }

    function selectUrls(urls) {
        thumbnailModel.selectUrls(urls)
    }

    //强制重新刷新整个缩略图界面
    function fouceUpdate() {
        if (main && visible) {
            gridView.bRefresh = !gridView.bRefresh
        }
    }

    // 获取列表中项的个数
    function count()
    {
        return thumbnailModel.rowCount()
    }

    // 获取列表中所有原始路径
    function allPaths() {
        return thumbnailModel.allPaths()
    }

    function allUrls() {
        return thumbnailModel.allUrls()
    }

    function executeViewImage() {
        ThumbnailTools.executeViewImage()
    }

    function runDeleteImg() {
        if (!visible)
            return
        ThumbnailTools.executeDelete()
    }

    //统计当前页面的缩略图时间范围
    function totalTimeScope() {
        if(thumnailListType === Album.Types.ThumbnailAllCollection &&
                GStatus.currentViewType === Album.Types.ViewCollecttion &&
                GStatus.currentCollecttionViewIndex === 3) { //仅在合集模式的时候激活计算，以此节省性能
            var visilbeIndexs = gridView.rectIndexes(0, 0, gridView.width, gridView.height)
            if (visilbeIndexs.length > 0) {
                var url1 = thumbnailModel.data(visilbeIndexs[0], "url")
                var url2 = thumbnailModel.data(visilbeIndexs[visilbeIndexs.length - 1], "url")
                var str = albumControl.getFileTime(url1, url2)
                timeChanged(str)
            } else {
                timeChanged("")
            }
        }
    }

    // 日视图/已导入视图，双击打开图片时，需要传入坐标，映射到点击Item上，才能准确显示大图预览动画初始位置
    function viewImageFromOuterDbClick(x,y) {
        var item = gridView.itemAt(x, y)
        var c = item.mapToItem(mainStack, 0, 0)
        console.log("c.x:", c.x, "c.y:", c.y)
        ThumbnailTools.executeViewImage(c.x, c.y, item.widht, item.height)
    }

   // 提供给合集日视图和已导入视图使用，用来刷新
   function flushRectSel(x,y,w,h,ctrl,mousePress, inPress) {
       if (gridView.contains(Qt.point(x,y)) && gridView.contains(Qt.point(x+w, y+h)) && w !== 0 && h !== 0) {
           // 按住Ctrl，鼠标点击事件释放时，处理点选逻辑
           if (ctrl && mousePress) {
               if (!inPress) {
                   var rectSel = gridView.rectIndexes(x, y + gridView.contentY, w, h)
                   if (rectSel.length === 1) {
           //不直接往theView.ism里面push，是为了触发onIsmChanged
                       var tempIsm = gridView.rectSelIndexes
                       if (tempIsm === undefined || tempIsm === null) {
                           gridView.rectSelIndexes = rectSel
                       } else {
                           if (gridView.rectSelIndexes.indexOf(rectSel[0]) === -1) {
                               tempIsm.push(rectSel[0])
                           } else {
                               tempIsm.splice(tempIsm.indexOf(rectSel[0]), 1)
                           }
                           gridView.rectSelIndexes = tempIsm
                       }
                   }
               }
           } else {
               gridView.rectangleSelect(x, y + gridView.contentY, w, h)
           }
       } else {
           // 1.按住ctrl，鼠标垮区域框选时，需要清空框选区域外的已选项
           // 2.不按住ctrl，鼠标单选，需要清空其他列表的已选项
           if (!ctrl && mousePress || ctrl && !mousePress)
               gridView.rectSelIndexes = []
       }

       // 跨区域框选后，需要手动激活列表焦点，这样快捷键才能生效
       if (thumbnailModel.selectedIndexes.length > 0) {
           gridView.forceActiveFocus()
       }

       selectedChanged()
   }

    Connections {
        target: thumbnailImage
        function onEscKeyPressed() {
            if (haveSelect) {
                selectAll(false)
            }
        }
    }

    Album.MouseEventListener {
        id: listener

        anchors { fill: parent }

        property Item pressedItem: null
        property int pressX: -1
        property int pressY: -1
        property variant cPress: null
        property bool bDbClicked: false

        acceptedButtons: { Qt.LeftButton | Qt.RightButton }

        hoverEnabled: true

        onPressXChanged: {
            cPress = mapToItem(gridView.contentItem, pressX, pressY)
        }

        onPressYChanged: {
            cPress = mapToItem(gridView.contentItem, pressX, pressY);
        }

        onPressed: (mouse)=> {
            // 鼠标点击在垂直滚动条上，不处理点击事件
            if (mouse.x >= (parent.width - GStatus.thumbnailListRightMargin - GStatus.thumbnialListCellSpace)) {
                return;
            }

            gridView.forceActiveFocus()

            var cPos = mapToItem(gridView.contentItem, mouse.x, mouse.y)
            var tempItem = gridView.itemAt(cPos.x, cPos.y);
            if (tempItem)
                pressedItem = tempItem;
            else
                pressedItem = null;

            if (mouse.source === Qt.MouseEventSynthesizedByQt) {
                if (indexItem && indexItem.image) {
                    gridView.currentIndex = index;
                    pressedItem = indexItem;
                } else {
                    pressedItem = null;
                }
            }

            pressX = mouse.x;
            pressY = mouse.y;

            if (!pressedItem || pressedItem.blank) {
                if (!gridView.ctrlPressed && !GStatus.windowDisActived) {
                    gridView.currentIndex = -1;
                    thumbnailModel.clearSelection();
                }
            } else {
                var pos = mapToItem(pressedItem, mouse.x, mouse.y);

                if (pos.x <= pressedItem.width && pos.y <= pressedItem.height) {
                    if (gridView.shiftPressed && gridView.currentIndex != -1) {
                        positioner.setRangeSelected(gridView.anchorIndex, pressedItem.index);
                    } else {
                        if (!gridView.ctrlPressed && !thumbnailModel.isSelected(positioner.map(pressedItem.index))) {
                            thumbnailModel.clearSelection();
                        }

                        if (thumnailListType !== Album.Types.ThumbnailDate) {
                            if (gridView.ctrlPressed) {
                                thumbnailModel.toggleSelected(positioner.map(pressedItem.index));
                            } else {
                                thumbnailModel.setSelected(positioner.map(pressedItem.index));
                            }
                        }
                    }

                    gridView.currentIndex = pressedItem.index;
                }
            }

            if (mouse.buttons & Qt.RightButton) {
                clearPressState();
                if (GStatus.currentViewType !== Album.Types.ViewDevice && haveSelect)
                    thumbnailMenu.popup(mouse.x, mouse.y)

                mouse.accepted = false;
            }
        }

        onCanceled: pressCanceled()

        onReleased: (mouse)=> {
            // 多选后，单选逻辑处理
            if (pressedItem && !pressedItem.blank && mouse.button !== Qt.RightButton && !main.rubberBand) {
                var pos = mapToItem(pressedItem, mouse.x, mouse.y);
                if ((pos.x <= pressedItem.width && pos.y <= pressedItem.height)
                        && (!(gridView.shiftPressed && gridView.currentIndex != -1) && !gridView.ctrlPressed) && haveSelectCount > 1) {
                    thumbnailModel.clearSelection();
                    thumbnailModel.setSelected(positioner.map(pressedItem.index));
                }
            } else {
                if (!main.rubberBand && !GStatus.windowDisActived && !(Qt.ControlModifier & mouse.modifiers))
                    thumbnailModel.clearSelection()
                GStatus.windowDisActived = false
            }

            pressCanceled();
        }

        onPressAndHold: (mouse)=> {
            if (mouse.source === Qt.MouseEventSynthesizedByQt) {
                clearPressState();
                if (haveSelect) {
                    thumbnailMenu.popup(mouse.x, mouse.y)
                }
            }
        }

        onClicked: (mouse)=> {
            clearPressState();

            var cPos = mapToItem(gridView.contentItem, mouse.x, mouse.y);
            var clickedItem = gridView.itemAt(cPos.x, cPos.y)
            if (!clickedItem || clickedItem.blank || gridView.currentIndex == -1 || gridView.ctrlPressed || gridView.shiftPressed) {
                //eventGenerator.sendMouseEvent(window, Album.EventGenerator.MouseButtonPress, mouse.x, mouse.y, mouse.button, mouse.buttons, mouse.modifiers);
                return;
            }

            var pos = mapToItem(clickedItem, mouse.x, mouse.y);
            if (pos.x < 0 || pos.x > clickedItem.width || pos.y < 0 || pos.y > clickedItem.height) {
                thumbnailModel.clearSelection();

                return;
            }

            // 单击模式点击/双击模式双击打开图片
            if (Qt.styleHints.singleClickActivation || bDbClicked || mouse.source === Qt.MouseEventSynthesizedByQt) {
                var c = clickedItem.mapToItem(mainStack, 0, 0)
                ThumbnailTools.executeViewImage(c.x, c.y, clickedItem.width, clickedItem.height)
            }
            else {
                bDbClicked = true;
                dbClickTimer.interval = Qt.styleHints.mouseDoubleClickInterval;
                dbClickTimer.start();
            }
        }

        onPositionChanged: (mouse)=> {
            gridView.ctrlPressed = (mouse.modifiers & Qt.ControlModifier);
            gridView.shiftPressed = (mouse.modifiers & Qt.ShiftModifier);

            var cPos = mapToItem(gridView.contentItem, mouse.x, mouse.y);
            var item = gridView.itemAt(cPos.x, cPos.y);
            var leftEdge = Math.min(gridView.contentX, gridView.originX);

            // 触发自动滚动框选
            if (pressX != -1) {
                gridView.scrollUp = (mouse.y <= 0 && gridView.contentY > 0);
                gridView.scrollDown = (mouse.y >= gridView.height
                    && gridView.contentY < gridView.contentItem.height - gridView.height);
            }

            // 更新框选区域
            if (main.rubberBand) {
                var rB = main.rubberBand;
                var ceil = 0;
                if (cPos.x < cPress.x) {
                    rB.x = Math.max(leftEdge, cPos.x);
                    rB.width = Math.abs(rB.x - cPress.x);
                } else {
                    rB.x = cPress.x;
                    ceil = Math.max(gridView.width - GStatus.thumbnailListRightMargin - GStatus.thumbnialListCellSpace, gridView.contentItem.width - GStatus.thumbnailListRightMargin - GStatus.thumbnialListCellSpace) + leftEdge;
                    rB.width = Math.min(ceil - rB.x, Math.abs(rB.x - cPos.x));
                }

                if (cPos.y < cPress.y) {
                    rB.y = Math.max(0, cPos.y);
                    rB.height = Math.abs(rB.y - cPress.y);
                } else {
                    rB.y = cPress.y;
                    ceil = Math.max(gridView.height, gridView.contentItem.height);
                    rB.height = Math.min(ceil - rB.y, Math.abs(rB.y - cPos.y));
                }

                // 确保宽高至少1像素
                rB.width = Math.max(1, rB.width);
                rB.height = Math.max(1, rB.height);

                //console.log("rb x:", rB.x, "y:", rB.y, "width:",rB.width, "height:", rB.height)
                gridView.rectangleSelect(rB.x, rB.y, rB.width, rB.height);
                return;
            }

            // 鼠标正在拖动，构建和显示框选框
            if (pressX != -1 && global.isDrag(pressX, pressY, mouse.x, mouse.y)) {
                thumbnailModel.pinSelection();
                main.rubberBand = rubberBandObject.createObject(gridView.contentItem, {x: cPress.x, y: cPress.y})
                gridView.interactive = false;
            }
        }

        Component {
            id: rubberBandObject

            Album.RubberBand {
                id: rubberBand

                width: 0
                height: 0
                z: 99999

                function close() {
                    opacityAnimation.restart()
                }

                OpacityAnimator {
                    id: opacityAnimation
                    target: rubberBand
                    to: 0
                    from: 1
                    duration: 200 * 0.5

                    easing {
                        bezierCurve: [0.4, 0.0, 1, 1]
                        type: Easing.Bezier
                    }

                    onStopped: {
                        rubberBand.visible = false
                        rubberBand.enabled = false
                        rubberBand.destroy()
                    }
                }
            }
        }

        function pressCanceled() {
            if (main.rubberBand) {
                main.rubberBand.close()
                main.rubberBand = null

                gridView.interactive = true;
                gridView.rectSelIndexes = null;
                thumbnailModel.unpinSelection();
            }

            clearPressState();
            gridView.cancelAutoscroll();
        }

        function clearPressState() {
            pressedItem = null;
            pressX = -1;
            pressY = -1;
        }

        Timer {
            id: dbClickTimer

            onTriggered: {
                listener.bDbClicked = false;
            }
        }

        GridView {
            id: gridView
            anchors.fill: parent
            clip: true
            interactive: enableMouse

            property int iconSize: 40

            property int anchorIndex: 0
            property bool ctrlPressed: false
            property bool shiftPressed: false

            property bool scrollUp: false
            property bool scrollDown: false

            property bool bRefresh: false
            property variant rectSelIndexes: null
            // 滚动滑块Delta值
            property int scrollDelta: 60
            currentIndex: -1

            keyNavigationWraps: false
            boundsBehavior: Flickable.StopAtBounds
            flickDeceleration: 10000

            //激活滚动条
            ScrollBar.vertical: ScrollBar {
                id: vbar
                active: false
                onPositionChanged: {
                    totalTimeScope()
                }
            }

            cellWidth: itemWidth
            cellHeight: itemHeight

            delegate: ThumbnailListDelegate {
                id: thumbnailListDelegate
                modelData: model
                m_index: index
                width: gridView.cellWidth - GStatus.thumbnialListCellSpace
                height: gridView.cellHeight - GStatus.thumbnialListCellSpace

                onBHoveredChanged: {
                    listener.bDbClicked = false
                }
            }

            // qml子控件消化鼠标移动事件，以便平台插件能够正常处理鼠标拖动事件，解决拖拽列表空隙或空白处，会拖动主界面的问题
            MouseArea {
                anchors.fill: parent
                propagateComposedEvents: true
                onPositionChanged: (mouse)=> {
                    mouse.accepted = true
                }

                onWheel: (wheel)=> {
                    if (!enableMouse) {
                        wheel.accepted = false
                    }

                    var datla = wheel.angleDelta.y / 2
                    if (Qt.ControlModifier & wheel.modifiers) {
                        // 按住ctrl，缩放缩略图
                        var curValue = statusBar.sliderValue
                        if (datla > 0)
                            statusBar.setSliderWidgetValue(curValue + 1)
                        else
                            statusBar.setSliderWidgetValue(curValue - 1)
                        wheel.accepted = true
                    } else {
                        gridView.executeScrollBar(datla)
                        wheel.accepted = false
                    }
                }
            }

            onContentYChanged: {

                vbar.active = true;

                if (contentY == 0) {
                    scrollUp = false;
                }

                if (contentY == contentItem.height - height) {
                    scrollDown = false;
                }

                // 更新框选框区域
                if (main.rubberBand) {
                    var rB = main.rubberBand;

                    if (scrollUp) {
                        rB.y = 0;
                        rB.height = listener.cPress.y;
                    }

                    if (scrollDown) {
                        var lastRow = gridView.contentY + gridView.height;
                        rB.height = lastRow - rB.y;
                    }

                    gridView.rectangleSelect(rB.x, rB.y, rB.width, rB.height);
                }
            }

            onScrollUpChanged: {
                if (scrollUp && gridView.visibleArea.heightRatio < 1.0) {
                    smoothY.enabled = true;
                    contentY = 0;
                } else {
                    contentY = contentY;
                    smoothY.enabled = false;
                }
            }

            onScrollDownChanged: {
                if (scrollDown && gridView.visibleArea.heightRatio < 1.0) {
                    smoothY.enabled = true;
                    contentY = contentItem.height - height;
                } else {
                    contentY = contentY;
                    smoothY.enabled = false;
                }
            }

            onCurrentIndexChanged: {
                positionViewAtIndex(currentIndex, GridView.Contain);
            }

            onRectSelIndexesChanged: {
                if (rectSelIndexes == null) {
                    return;
                }

                if (rectSelIndexes.length) {
                    currentIndex = rectSelIndexes[0];
                }

                thumbnailModel.updateSelection(positioner.maps(rectSelIndexes), gridView.ctrlPressed);
            }

            function updateSelection(modifier) {
                if (modifier & Qt.ShiftModifier) {
                    positioner.setRangeSelected(anchorIndex, currentIndex);
                } else {
                    thumbnailModel.clearSelection();
                    thumbnailModel.setSelected(positioner.map(currentIndex));
                }
            }

            function cancelAutoscroll() {
                scrollUp = false;
                scrollDown = false;
            }

            // 框选指定区域内容的项目
            function rectangleSelect(x, y, width, height) {
                gridView.rectSelIndexes = rectIndexes(x, y, width, height);
            }

            // 获取指定矩形区域内项目索引
            function rectIndexes(x, y, w, h) {
                var rB = null
                if (main.rubberBand)
                    rB = main.rubberBand
                else {
                    var topleft = mapToItem(gridView.contentItem, x, y)
                    rB = rubberBandObject.createObject(gridView.contentItem, {x: topleft.x, y: topleft.y})
                    rB.visible = false
                    rB.width = w
                    rB.height = h
                }

                var rows = (gridView.flow == GridView.FlowLeftToRight);
                var axis = rows ? gridView.width : gridView.height;
                var step = rows ? cellWidth : cellHeight;
                var perStripe = Math.floor(axis / step);
                var stripes = Math.ceil(gridView.count / perStripe);
                var cWidth = gridView.cellWidth;
                var cHeight = gridView.cellHeight;
                var midWidth = gridView.cellWidth / 2;
                var midHeight = gridView.cellHeight / 2;
                var indices = [];

                if (gridView.width > 0 && isFinite(step)) {
                    for (var s = 0; s < stripes; s++) {
                        for (var i = 0; i < perStripe; i++) {
                            var index = (s * perStripe) + i;

                            if (index >= gridView.count) {
                                break;
                            }

                            if (positioner.isBlank(index)) {
                                continue;
                            }

                            var itemX = ((rows ? i : s) * gridView.cellWidth);
                            var itemY = ((rows ? s : i) * gridView.cellHeight);

                            if (gridView.effectiveLayoutDirection == Qt.RightToLeft) {
                                itemX -= (rows ? gridView.contentX : gridView.originX);
                                itemX += cWidth;
                                itemX = (rows ? gridView.width : gridView.contentItem.width) - itemX;
                            }

                            // 框选框与item项相交，即认为选中
                            if (rB.intersects(Qt.rect(itemX, itemY,
                                                      cWidth, cHeight))) {
                                var item = gridView.contentItem.childAt(itemX + midWidth, itemY + midHeight);

                                indices.push(index);
                            }
                        }
                    }
                }
                if (main.rubberBand == null)
                    rB.close()

                return indices
            }

            function executeScrollBar(delta) {
                if (enableMouse && visible) {
                    if (gridView.contentHeight <= gridView.height)
                        return

                    vbar.active = true
                    gridView.contentY -= delta
                    if(gridView.contentY < 0) {
                        gridView.contentY = 0
                    } else if(gridView.contentY > gridView.contentHeight - gridView.height) {
                        gridView.contentY = gridView.contentHeight - gridView.height
                    }
                } else {
                    vbar.active = false
                }
            }

            Behavior on contentY { id: smoothY; enabled: false; SmoothedAnimation { velocity: 700 } }

            Keys.onPressed: (event)=> {
                switch (event.key) {
                case Qt.Key_Return:
                case Qt.Key_Enter:
                    if (menuItemStates.canView)
                        ThumbnailTools.executeViewImageCutSwitch()
                    break
                case Qt.Key_Home:
                    currentIndex = 0
                    updateSelection(event.modifiers)
                    break
                case Qt.Key_End:
                    currentIndex = count - 1
                    updateSelection(event.modifiers)
                    break
                case Qt.Key_Period:
                    if (!menuItemStates.isInTrash && !menuItemStates.isInDevice) {
                        if (menuItemStates.canFavorite)
                            ThumbnailTools.executeFavorite()
                        else
                            ThumbnailTools.executeUnFavorite()
                    }
                    break
                case Qt.Key_Shift:
                    shiftPressed = true
                    if(currentIndex != -1)
                        anchorIndex = currentIndex
                    break
                default:
                    break
                }
            }

            Keys.onReleased: (event)=> {
                switch (event.key) {
                case Qt.Key_Shift:
                    shiftPressed = false
                    anchorIndex = 0
                    break
                default:
                    break       
                }
            }
        }

        Album.ThumbnailModel {
            id: thumbnailModel

            onSrcModelReseted: {
                if (!gridView.model) {
                    gridView.model = positioner;
                    gridView.currentIndex = -1;
                }
            }
        }

        Album.Positioner {
            id: positioner

            enabled: true

            thumbnailModel: thumbnailModel

            perStripe: Math.floor(((gridView.flow == GridView.FlowLeftToRight)
                ? gridView.width : gridView.height) / ((gridView.flow == GridView.FlowLeftToRight)
                ? gridView.cellWidth : gridView.cellHeight));
        }

        Album.ItemViewAdapter {
            id: viewAdapter

            adapterView: gridView
            adapterModel: positioner
            adapterIconSize: gridView.iconSize * 2
            adapterVisibleArea: Qt.rect(gridView.contentX, gridView.contentY, gridView.contentWidth, gridView.contentHeight)

            Component.onCompleted: {
                gridView.movementStarted.connect(viewAdapter.viewScrolled);
                thumbnailModel.viewAdapter = viewAdapter;
            }
        }
    }

    Connections {
        target: GStatus
        function onSigSelectAll() {
            if (gridView.visible)
                selectAll(bSel)
        }

        function onSigPageUp() {
            if (gridView.visible) {
                gridView.executeScrollBar(gridView.scrollDelta)
            }
        }

        function onSigPageDown() {
            if (gridView.visible) {
                gridView.executeScrollBar(-gridView.scrollDelta)
            }
        }
    }

    //缩略图菜单
    //注意：涉及界面切换的，需要做到从哪里进来，就退出到哪里
    //菜单显隐逻辑有点绕，建议头脑清醒的时候再处理
    Menu {
        id: thumbnailMenu
        maxVisibleItems: 30
        //显示大图预览
        RightMenuItem {
            text: qsTr("View")
            visible: menuItemStates.canView
            onTriggered: {
                ThumbnailTools.executeViewImageCutSwitch()
            }
        }

        //全屏预览
        RightMenuItem {
            text: qsTr("Fullscreen")
            visible:  menuItemStates.canFullScreen
            onTriggered: {
                ThumbnailTools.executeFullScreen()
            }
            Shortcut {
                enabled: gridView.activeFocus && menuItemStates.canFullScreen
                autoRepeat: false
                sequence : "F11"
                onActivated : {
                    ThumbnailTools.executeFullScreen()
                }
            }
        }

        //调起打印接口
        RightMenuItem {
            text: qsTr("Print")
            visible: menuItemStates.canPrint

            onTriggered: {
                ThumbnailTools.executePrint()
            }

            Shortcut {
                enabled: gridView.activeFocus && menuItemStates.canPrint
                autoRepeat: false
                sequence : "Ctrl+P"
                onActivated : {
                    ThumbnailTools.executePrint()
                }
            }
        }

        //幻灯片
        RightMenuItem {
            text: qsTr("Slide show")
            visible: menuItemStates.canSlideShow
            onTriggered: {
                ThumbnailTools.excuteSlideShow()
            }

            Shortcut {
                enabled: gridView.activeFocus && menuItemStates.canSlideShow
                autoRepeat: false
                sequence : "F5"
                onActivated : {
                    ThumbnailTools.excuteSlideShow()
                }
            }
        }

        MenuSeparator {
            visible: thumnailListType !== Album.Types.ThumbnailTrash
            height: visible ? GStatus.rightMenuSeparatorHeight : 0
        }

        //添加到相册子菜单
        //隐藏交给后面的Component.onCompleted解决
        Menu {
            enabled: thumnailListType !== Album.Types.ThumbnailTrash
            id: addToAlbum
            title: qsTr("Add to album")

            RightMenuItem {
                text: qsTr("New album")
                onTriggered: {
                    newAlbum.isChangeView = false
                    newAlbum.importSelected = true
                    newAlbum.setNormalEdit()
                    newAlbum.show()
                }
            }

            MenuSeparator {
            }

            Repeater {
                id: recentFilesInstantiator
                property bool bRreshEnableState: false
                model: {
                    GStatus.albumChangeList
                    albumControl.getAllCustomAlbumId().length
                }
                delegate: RightMenuItem {
                    text: {
                        GStatus.albumChangeList
                        return albumControl.getAllCustomAlbumName()[index]
                    }
                    enabled: {
                        recentFilesInstantiator.bRreshEnableState
                        return albumControl.canAddToCustomAlbum(albumControl.getAllCustomAlbumId()[index], GStatus.selectedPaths)
                    }
                    onTriggered:{
                        // 获取所选自定义相册的Id，根据Id添加到对应自定义相册
                        var customAlbumId = albumControl.getAllCustomAlbumId()[index]
                        albumControl.insertIntoAlbum(customAlbumId , GStatus.selectedPaths)
                        DTK.sendMessage(thumbnailImage, qsTr("Successfully added to “%1”").arg(albumControl.getAllCustomAlbumName(GStatus.albumChangeList)[index]), "notify_checked")
                        recentFilesInstantiator.bRreshEnableState = !recentFilesInstantiator.bRreshEnableState
                    }
                }
            }
        }

        //导出图片为其它格式
        RightMenuItem {
            text: qsTr("Export")
            visible: thumnailListType !== Album.Types.ThumbnailTrash
                     && ((selectedUrls.length === 1 && FileControl.pathExists(selectedUrls[0]) && menuItemStates.haveImage) || !menuItemStates.haveVideo)
            onTriggered: {
                ThumbnailTools.excuteExport()
            }

            Shortcut {
                enabled: gridView.activeFocus && menuItemStates.canExport
                autoRepeat: false
                sequence : "Ctrl+E"
                onActivated : {
                    ThumbnailTools.excuteExport()
                }
            }
        }

        //复制图片
        RightMenuItem {
            text: qsTr("Copy")
            visible: menuItemStates.canCopy
            onTriggered: {
                ThumbnailTools.executeCopy()
            }
        }

        //删除图片
        RightMenuItem {
            text: qsTr("Delete")
            visible: menuItemStates.canDelete && !menuItemStates.isInDevice
            onTriggered: {
                deleteDialog.setDisplay(thumnailListType === Album.Types.ThumbnailTrash ? Album.Types.TrashSel : Album.Types.TrashNormal, selectedPaths.length)
                deleteDialog.show()
            }
        }

        //从相册移除（只在自定义相册中显示）
        RightMenuItem {
            text: qsTr("Remove from album")
            visible: thumnailListType === Album.Types.ThumbnailCustomAlbum
            onTriggered: {
                ThumbnailTools.executeRemoveFromAlbum()
            }
        }

        MenuSeparator {
            visible: thumnailListType !== Album.Types.ThumbnailTrash
            height: visible ? GStatus.rightMenuSeparatorHeight : 0
        }

        //添加到我的收藏
        RightMenuItem {
            id: favoriteAction
            text: qsTr("Favorite")
            visible: !menuItemStates.isInTrash && !menuItemStates.isInDevice && menuItemStates.canFavorite
            onTriggered: {
                ThumbnailTools.executeFavorite()
            }
        }

        //从我的收藏中移除
        RightMenuItem {
            id: unFavoriteAction
            text: qsTr("Unfavorite")
            visible: !menuItemStates.isInTrash && !menuItemStates.isInDevice && !menuItemStates.canFavorite
            onTriggered: {
                ThumbnailTools.executeUnFavorite()
            }
        }

        MenuSeparator {
            visible: menuItemStates.canRotate
            height: visible ? GStatus.rightMenuSeparatorHeight : 0
        }

        //顺时针旋转
        RightMenuItem {
            text: qsTr("Rotate clockwise")
            visible: menuItemStates.canRotate
            onTriggered: {
                ThumbnailTools.executeRotate(90)
            }
        }

        //逆时针旋转
        RightMenuItem {
            text: qsTr("Rotate counterclockwise")
            visible: menuItemStates.canRotate
            onTriggered: {
                ThumbnailTools.executeRotate(-90)
            }
        }

        //设置为壁纸
        RightMenuItem {
            text: qsTr("Set as wallpaper")
            id: setAsWallpaperAction
            visible: menuItemStates.canWallpaper
            onTriggered: {
                ThumbnailTools.executeSetWallpaper()
            }

            Shortcut {
                enabled: gridView.activeFocus && menuItemStates.canWallpaper
                autoRepeat: false
                sequence : "Ctrl+F9"
                onActivated : {
                    ThumbnailTools.executeSetWallpaper()
                }
            }
        }

        //在文件管理器中显示
        RightMenuItem {
            text: qsTr("Display in file manager")
            id: displayInFileManagerAction
            visible: menuItemStates.canDisplayInFolder
            onTriggered: {
                ThumbnailTools.executeDisplayInFileManager()
            }

            Shortcut {
                enabled: gridView.activeFocus && menuItemStates.canDisplayInFolder
                autoRepeat: false
                sequence : "Alt+D"
                onActivated : {
                    ThumbnailTools.executeDisplayInFileManager()
                }
            }
        }

        //恢复
        RightMenuItem {
            text: qsTr("Restore")
            visible: thumnailListType === Album.Types.ThumbnailTrash
            onTriggered: {
                ThumbnailTools.executeRestore()
            }
        }

        //照片信息
        RightMenuItem {
            text: qsTr("Photo info")
            id: photoInfoAction
            visible: menuItemStates.canViewPhotoInfo
            onTriggered: {
                ThumbnailTools.executeViewPhotoInfo()
            }

            Shortcut {
                enabled: gridView.activeFocus && menuItemStates.canViewPhotoInfo
                autoRepeat: false
                sequence : "Ctrl+I"
                onActivated : {
                    ThumbnailTools.executeViewPhotoInfo()
                }
            }
        }

        //视频信息
        RightMenuItem {
            text: qsTr("Video info")
            id: videoInfoAction
            visible: menuItemStates.canViewVideoInfo
            onTriggered: {
                ThumbnailTools.executeViewVideoInfo()
            }

            Shortcut {
                enabled: gridView.activeFocus && menuItemStates.canViewVideoInfo
                autoRepeat: false
                sequence : "Ctrl+I"
                onActivated : {
                    ThumbnailTools.executeViewVideoInfo()
                }
            }
        }

        Component.onCompleted: {
            //最近删除界面下隐藏添加到相册的子菜单
            if(thumnailListType === Album.Types.ThumbnailTrash) {
                var item = itemAt(5)
                item.visible = false
                item.height = 0
            }
        }
    }

    Component.onCompleted: {
        GStatus.sigThumbnailStateChange.connect(fouceUpdate)
        deleteDialog.sigDoDeleteImg.connect(runDeleteImg)

        // 缩略图列表模型选中内容有变化，向外发送选中内容改变信号，以便外部维护全局选中队列GStatus.selectedPaths
        thumbnailModel.selectedIndexesChanged.connect(selectedChanged)
    }

    onRealCellWidthChanged: {
        totalTimeScope()
    }

}
