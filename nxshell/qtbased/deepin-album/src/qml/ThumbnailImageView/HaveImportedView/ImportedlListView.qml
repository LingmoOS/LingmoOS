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

import "../../Control/ListView"
import "../../Control"
import "../../"
Item {
    id : importedListView
    signal sigUnSelectAll()
    signal sigListViewPressed(int x, int y)
    signal sigListViewReleased(int x, int y)

    property var selectedPaths: []
    property int filterComboOffsetY: 5
    property int spaceCtrlHeight: filterCombo.y + filterComboOffsetY
    property int importCheckboxHeight: 26
    property int listMargin: 10 // 已导入列表子项上、下边距
    property int rowSizeHint: (width - GStatus.thumbnailListRightMargin) / GStatus.cellBaseWidth
    property real realCellWidth : (width - GStatus.thumbnailListRightMargin) / rowSizeHint
    property bool isFirstLoad: true

    signal sigTextUpdated(string str)

    property int topDelegateIndex: 0
    property int topDelegateIndexTmp: 0
    property bool checkBoxClicked: false

    //view依赖的model管理器
    property ListModel importedListModel: ListModel {
        id: theModel
        property var selectedPathObjs: []
        property var dayHeights: []
        function loadImportedInfos() {
            theModel.clear()
            theModel.selectedPathObjs = []
            theModel.dayHeights = []
            // 从后台获取所有已导入数据,倒序
            var titleInfos = []
            if (Number(FileControl.getConfigValue("", "loadImport", 1)))
                titleInfos = albumControl.getAllImportTimelinesTitle(filterCombo.currentIndex);
            var i = 0
            var dayHeight = 0
            var listHeight = 0
            theView.listContentHeight = 0
            var titlePaths
            for (var j = 0; j < titleInfos.length; j++) {
                theModel.append({"title": titleInfos[j]})

                // 当前标题列表选中数据初始化
                titlePaths = []
                var selectedPathObj = {"id": j, "paths": titlePaths}
                theModel.selectedPathObjs.push(selectedPathObj)

                // 计算每个标题列表高度
                titlePaths = albumControl.getTimelinesTitlePaths(titleInfos[j], filterCombo.currentIndex)
                listHeight = Math.abs(Math.ceil(titlePaths.length / Math.floor(importedListView.width / realCellWidth)) * realCellWidth)
                dayHeight = listHeight + listMargin * 2 + importCheckboxHeight + (j === 0 ? spaceCtrlHeight : 0)
                dayHeights.push(dayHeight)
                theView.listContentHeight += dayHeight

            }
        }
    }

    // 刷新已导入列表已选路径
    function updateSelectedPaths()
    {
        var tmpPaths = []
        for (var i = 0; i < theModel.selectedPathObjs.length; i++) {
            if (theModel.selectedPathObjs[i].paths.length > 0) {
                for (var j = 0; j < theModel.selectedPathObjs[i].paths.length; j++)
                    tmpPaths.push(theModel.selectedPathObjs[i].paths[j])
            }
        }

        selectedPaths = tmpPaths
        if (importedListView.visible) {
            GStatus.selectedPaths = selectedPaths
        }
    }

    //获取当前处于top位置的delegate索引，并发送更新信号
    function importTimeScope() {
        var topDelegateIndexTmp = theView.indexAt(0, theView.contentY)
        if (topDelegateIndex !== topDelegateIndexTmp) { // 如果索引发生变化
            topDelegateIndex = topDelegateIndexTmp; // 更新属性
            topDelegateIndexChanged(topDelegateIndex); // 发送信号
        }
    }

    //已导入列表本体
    ListView {
        id: theView
        clip: true
        interactive: false //禁用原有的交互逻辑，重新开始定制
        model: theModel
        width: parent.width
        height: parent.height
        delegate: importedListDelegate

        //鼠标正在按下状态
        property bool inPress: theMouseArea.pressedButtons & Qt.LeftButton
        //框选滚动方向
        property var scrollDirType: Album.Types.NoType
        property var listContentHeight
        property int rectSelScrollOffset: GStatus.rectSelScrollStep
        signal dbClicked(string url)

        //激活滚动条
        ScrollBar.vertical: ScrollBar {
            id: vbar
            active: true

            // 通知外部组件当前顶部的索引发生了变化
            signal topDelegateIndexChanged(int newIndex)
            signal importedLabelTextChanged(string str)

            onPositionChanged: {
                var topDelegateIndexTmp = theView.indexAt(0, theView.contentY)
                if (topDelegateIndex !== topDelegateIndexTmp) { // 如果索引发生变化
                    topDelegateIndex = topDelegateIndexTmp; // 更新属性
                    topDelegateIndexChanged(topDelegateIndex); // 发送信号
                }
            }

        }

        Connections {
            target: vbar
            function onTopDelegateIndexChanged(newIndex) {
                var firstItemIndex = topDelegateIndex; // 使用当前顶部的索引
                var firstItemLabel = theModel.get(firstItemIndex).title;

                var item = theView.itemAtIndex(firstItemIndex);
                if (item) {
                    var viewCount = item.count;
                }
                var firstItemCount = viewCount ; //importedGridView.count();
                var labelText = qsTr("Imported on") + " " + firstItemLabel + " " + (firstItemCount === 1 ? qsTr("1 item") : qsTr("%1 items").arg(firstItemCount));
                sigTextUpdated(labelText);
                importedLabelTextChanged(" ")
            }
        }

        MouseArea {
            id: theMouseArea
            // 鼠标区域需要包含空白区域，否则点击空白区域会拖动相册应用
            anchors.fill: parent.contentHeight > parent.height ? parent.contentItem : parent
            acceptedButtons: Qt.LeftButton //仅激活左键
            propagateComposedEvents: true

            property bool ctrlPressed: false // 记录ctrl是否按下

            onClicked: (mouse)=> {
                //允许鼠标事件传递给子控件处理,否则鼠标点击缩略图收藏图标不能正常工作
                //同时propagateComposedEvents需设置为true
                //注意：不能传递onPressed、onReleased等基础事件，会有bug；合成事件onClicked等可以传递
                mouse.accepted = false
            }

            onPressed: (mouse)=> {
                if(mouse.button === Qt.RightButton) {
                    mouse.accepted = false
                    return
                }

                var gPos = theMouseArea.mapToGlobal(mouse.x, mouse.y)
                sigListViewPressed(gPos.x, gPos.y)
                if (checkBoxClicked) {
                    mouse.accepted = false
                    return
                }

                ctrlPressed = Qt.ControlModifier & mouse.modifiers

                theView.scrollDirType = Album.Types.NoType

                rubberBandImport.x1 = mouse.x
                rubberBandImport.y1 = mouse.y
                rubberBandImport.x2 = mouse.x
                rubberBandImport.y2 = mouse.y
            }

            onDoubleClicked: (mouse)=> {
                if (GStatus.selectedPaths.length > 0)
                    theView.dbClicked(GStatus.selectedPaths[0])

                rubberBandImport.clearRect()
            }

            onMouseXChanged: (mouse)=> {
                if(mouse.button === Qt.RightButton) {
                    mouse.accepted = false
                    return
                }

                if (parent.inPress) {
                    rubberBandImport.x2 = mouse.x
                }
            }

            onMouseYChanged: (mouse)=> {
                if(mouse.button === Qt.RightButton) {
                    mouse.accepted = false
                    return
                }

                // 刷新矩形第二锚点，内部触发updateRect，保证y2值标记为矩形底部坐标
                if (parent.inPress) {
                    rubberBandImport.y2 = mouse.y
                }

                // 确定滚动延展方向（向上还是向下）
                var parentY = mapToItem(theView, mouse.x, mouse.y).y
                if (parentY > theView.height) {
                    // 选择框超出ListView底部，ListView准备向下滚动
                    if (parent.contentHeight > parent.height)
                        theView.scrollDirType = Album.Types.ToBottom
                } else if (parentY < 0) {
                    // 选择框超出ListView顶部，ListView准备向上滚动
                    theView.scrollDirType = Album.Types.ToTop
                } else {
                    if (rectScrollTimer.running)
                        rectScrollTimer.stop()
                }

                var gPos = theMouseArea.mapToGlobal(mouse.x, mouse.y)
                sigListViewReleased(gPos.x, gPos.y)
            }

            onReleased: (mouse)=> {
                if(mouse.button === Qt.RightButton) {
                    mouse.accepted = false
                    return
                }

                // ctrl按下，鼠标点击事件释放时，需要再发送一次框选改变信号，用来在鼠标释放时实现ctrl取消选中的功能
                if ((Qt.ControlModifier & mouse.modifiers) && rubberBandImport.width < 3 & rubberBandImport.height < 3) {
                    rubberBandImport.rectSelChanged()
                }

                ctrlPressed = false

                theView.scrollDirType = Album.Types.NoType
                rubberBandImport.clearRect()
            }
            onWheel: (wheel)=> {
                // 滚动时，激活滚动条显示
                vbar.active = true
                var datla = wheel.angleDelta.y
                if (Qt.ControlModifier & wheel.modifiers) {
                    // 按住ctrl，缩放缩略图
                    var curValue = statusBar.sliderValue
                    if (datla > 0)
                        statusBar.setSliderWidgetValue(curValue + 1)
                    else
                        statusBar.setSliderWidgetValue(curValue - 1)
                } else {
                    // 正常滚动显示缩略图内容
                    if( datla > 0 ) {
                        vbar.decrease()
                    } else {
                        vbar.increase()
                    }
                }
            }

            //橡皮筋控件
            RubberBand {
                id: rubberBandImport
                visible: theView.inPress
            }

            Timer {
                id: rectScrollTimer
                interval: 100
                running: theView.scrollDirType !== Album.Types.NoType
                repeat: true
                onTriggered: {
                    // 选择框向下延展滚动
                    if (theView.scrollDirType === Album.Types.ToBottom) {
                        var newY2 = rubberBandImport.y2 + theView.rectSelScrollOffset
                        if (newY2 <= theView.listContentHeight) {
                            rubberBandImport.y2 = newY2
                            theView.contentY = theView.contentY + theView.rectSelScrollOffset + theView.originY
                        } else {
                            // 选择框底部最大值为内容区域底部
                            theView.contentY = theView.listContentHeight - theView.height
                            rubberBandImport.y2 = theView.listContentHeight
                            rectScrollTimer.stop()
                        }
                    } else if (theView.scrollDirType === Album.Types.ToTop) {
                        if (rubberBandImport.top() < 0) {
                            rectScrollTimer.stop()
                            return
                        }

                        // 矩形顶部向上延展
                        if (theView.contentY <= rubberBandImport._bottom() || rubberBandImport._bottom() === rubberBandImport._top()) {
                            var newTop = rubberBandImport._top() - theView.rectSelScrollOffset
                            if (newTop > 0) {
                                rubberBandImport.y2 = newTop
                                theView.contentY = theView.contentY - theView.rectSelScrollOffset + theView.originY
                            } else {
                                // 选择框顶部最小值为内容区域顶部
                                rubberBandImport.y2 = 0
                                theView.contentY = 0 + theView.originY

                                rectScrollTimer.stop()
                            }
                        } else {
                            // 矩形框底部向上收缩
                            var newBottom = rubberBandImport._bottom() - theView.rectSelScrollOffset
                            if (newBottom > rubberBandImport._top()) {
                                rubberBandImport.y2 = newBottom
                                theView.contentY = theView.contentY - theView.rectSelScrollOffset + theView.originY
                            } else {
                                var srcollOffset = Math.abs(rubberBandImport.y1 - rubberBandImport.y2)
                                rubberBandImport.y2 = rubberBandImport.y1
                                theView.contentY = theView.contentY - srcollOffset + theView.originY
                            }
                        }
                    }
                }
            }
        }//MouseArea
    }

    Connections {
        target: GStatus
        function onSigPageUp() {
            if (visible) {
                vbar.active = true
                vbar.decrease()
            }
        }

        function onSigPageDown() {
            if (visible) {
                vbar.active = true
                vbar.increase()
            }
        }
    }

    //已导入列表代理控件
    Component {
        id: importedListDelegate

        Control {
            id :importControl
            z: 2
            width: theView.width
            height: importedGridView.height + importedListView.listMargin * 2 + importedListView.importCheckboxHeight + spaceRect.height
            property string m_index: index
            property var theViewTitle: global.objIsEmpty(theModel.get(index)) ? "" : theModel.get(index).title //日期标题文本内容
            property alias count: importedGridView.count

            Item {
                id: spaceRect
                width: parent.width
                height: index == 0 ? importedListView.spaceCtrlHeight : 0
            }

            CheckBox {
                id: importedCheckBox
                height: importedListView.importCheckboxHeight
                visible: selectedPaths.length > 0 && index > 0
                checked: importedGridView.haveSelectAll
                font: DTK.fontManager.t6
                anchors.top: (index == 0 ? spaceRect.bottom : spaceRect.top)
                //text: qsTr("Imported in %1   %2").arg(theViewTitle).arg(importedGridView.count() === 1 ? qsTr("1 item") : qsTr("%1 items").arg(importedGridView.count()))
                onClicked: {
                    if(checked) {
                        importedGridView.selectAll(true)
                    } else {
                        importedGridView.selectAll(false)
                    }
                }

                Connections {
                    target: importedListView
                    function onSigListViewPressed(x, y) {
                        var object = importedCheckBox.mapFromGlobal(x,y)
                        if (importedCheckBox.contains(object)) {
                            checkBoxClicked = true
                            if (importedCheckBox.checkState === Qt.Checked) {
                                importedCheckBox.checkState = Qt.Unchecked
                                importedGridView.selectAll(false)
                            } else {
                                importedCheckBox.checkState = Qt.Checked
                                importedGridView.selectAll(true)
                            }
                        }
                    }

                    function onSigListViewReleased(x, y) {
                        checkBoxClicked = false
                    }
                }
            }
            Label {
                anchors {
                    left :importedCheckBox.visible ? importedCheckBox.right : parent.left
                    top :importedCheckBox.top
                }
                topPadding: 1
                visible: index > 0
                font: DTK.fontManager.t6
                id: importedLabel
                z:4
                text: qsTr("Imported on") + " " + theViewTitle + " " + (importedGridView.count === 1 ? qsTr("1 item") : qsTr("%1 items").arg(importedGridView.count))

                Component.onCompleted: {
                    if (isFirstLoad) {
                        isFirstLoad = false
                        haveImportedListView.setDataRange(importedLabel.text)
                        // sigTextUpdated(importedLabel.text)
                        importedLabel.text = " "
                    }
                }
            }

            //缩略图网格表
            ThumbnailListViewAlbum {
                id: importedGridView
                anchors {
                    left: parent.left
                    top: importedCheckBox.bottom
                    topMargin: importedListView.listMargin
                    bottomMargin: importedListView.listMargin
                }
                enableMouse: false
                width: parent.width
                height: Math.abs(Math.ceil(importedGridView.count / Math.floor((parent.width) / itemWidth)) * itemHeight)
                thumnailListType: Album.Types.ThumbnailDate
                proxyModel.sourceModel: Album.ImageDataModel { id: dataModel; modelType: Album.Types.HaveImported}

                Connections {
                    target: rubberBandImport
                    function onRectSelChanged() {
                        var pos1 = theMouseArea.mapToItem(importedGridView, rubberBandImport._left(), rubberBandImport._top())
                        var pos2 = theMouseArea.mapToItem(importedGridView, rubberBandImport._right(), rubberBandImport._bottom())
                        var rectsel = albumControl.rect(pos1, pos2)
                        var rectList = Qt.rect(0, 0, importedGridView.width, importedGridView.height)
                        var rect = albumControl.intersected(rectList, rectsel)
                        var bDetectMousePrees = albumControl.manhattanLength(pos1, pos2) < 3 // 识别此次框选事件是否为鼠标点击事件，以便在列表控件处理ctrl按键相关的操作
                        importedGridView.flushRectSel(rect.x, rect.y, rect.width, rect.height, theMouseArea.ctrlPressed, bDetectMousePrees, theView.inPress)
                    }
                }

                // 监听缩略图子控件选中状态，一旦改变，更新已导入视图所有选中路径
                Connections {
                    target: importedGridView
                    function onSelectedChanged() {
                        if (index > -1) {
                            theModel.selectedPathObjs[index].paths = importedGridView.selectedUrls
                        }

                        updateSelectedPaths()
                    }
                }

                Connections {
                    target: importedListView
                    function onSigUnSelectAll() {
                        importedGridView.selectAll(false)
                    }
                }

                Connections {
                    target: theView
                    function onDbClicked(url) {
                        var openPaths = importedGridView.allUrls()
                        if (openPaths.indexOf(url) !== -1) {
                            var pos = theMouseArea.mapToItem(importedGridView, rubberBandImport._left(), rubberBandImport._top())
                            importedGridView.viewImageFromOuterDbClick(pos.x, pos.y)
                        }
                    }
                }

                function flushView() {
                    dataModel.importTitle = theViewTitle
                    importedGridView.proxyModel.refresh(filterCombo.currentIndex)
                }

                Component.onCompleted: {
                    importedGridView.flushView()
                    importedGridView.selectUrls(theModel.selectedPathObjs[index].paths)
                }
            }
        }
    }
}
