// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Window
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Dialogs
import org.deepin.dtk 1.0

import org.deepin.album 1.0 as Album

import "../../Control"
import "../../Control/ListView"
import "../../Control/Animation"
import "../../"

SwitchViewAnimation {
    id: dayView

    signal sigListViewPressed(int x, int y)
    signal sigListViewReleased(int x, int y)
    property int scrollDelta: 60
    property int timeLineLblHeight: 36
    property int timeLineLblMargin: 10
    property int selAllCheckBoxHeight: 22
    property int rowSizeHint: (width - GStatus.thumbnailListRightMargin) / GStatus.cellBaseWidth
    property real realCellWidth : (width - GStatus.thumbnailListRightMargin) / rowSizeHint
    property var dayHeights: []

    property var selectedPaths: []
    property string numLabelText: "" //总数标签显示内容
    property string selectedText: getSelectedText(selectedPaths)

    property int currentColletionIndex: collecttionView.currentViewIndex

    property bool checkBoxClicked: false
    property bool isFirstLoad: true
    property int topDelegateIndex: 0
    property int topDelegateIndexTmp: 0
    property alias count: theModel.count

    // 更新日期和照片数量信号
    signal numLabelTextUpdate(string str)
    signal timelineLabelTextUpdate(string str)

    function setTimeLine(str) {
        titleTimeLineLabel.text = str
    }

    function setNumLabel(str) {
        titlePhotoNumberLabel.text = str
    }

    Rectangle {
        anchors.fill : parent
        color: DTK.themeType === ApplicationHelper.LightType ? "#f8f8f8"
                                                              : "#202020"
    }

    Connections {
        target: collecttionView
        function onFlushDayViewStatusText() {
            if (visible) {
                if (selectedPaths.length > 0)
                    getSelectedText(selectedPaths)
                else
                    getNumLabelText()
            }
        }
    }

    Connections {
        target: albumControl
        function onSigRepeatUrls(urls) {
            if (visible && collecttionView.currentViewIndex === 2) {
                theView.sigUnSelectAll()
                selectedPaths = urls
                if (selectedPaths.length > 0)
                    getSelectedText(selectedPaths)
                else
                    getNumLabelText()
                GStatus.selectedPaths = selectedPaths
            }
        }

        //收到导入完成消息
        function onSigImportFinished() {
            if (visible) {
                //刷新数量显示
                getNumLabelText()
            }
        }
    }

    // 刷新总数标签
    function getNumLabelText() {
        //QML的翻译不支持%n的特性，只能拆成这种代码

        var photoCountText = ""
        var photoCount = albumControl.getAllInfoConut(1)
        if(photoCount === 0) {
            photoCountText = ""
        } else if(photoCount === 1) {
            photoCountText = qsTr("1 photo")
        } else {
            photoCountText = qsTr("%1 photos").arg(photoCount)
        }

        var videoCountText = ""
        var videoCount = albumControl.getAllInfoConut(2)
        if(videoCount === 0) {
            videoCountText = ""
        } else if(videoCount === 1) {
            videoCountText = qsTr("1 video")
        } else {
            videoCountText = qsTr("%1 videos").arg(videoCount)
        }

        numLabelText = photoCountText + (videoCountText !== "" ? ((photoCountText !== "" ? " " : "") + videoCountText) : "")

        if (visible) {
            GStatus.statusBarNumText = numLabelText
        }
    }

    // 刷新选中项目标签内容
    function getSelectedText(paths) {
        var selectedNumText = GStatus.getSelectedNumText(paths, numLabelText)
        if (visible)
            GStatus.statusBarNumText = selectedNumText
        return selectedNumText
    }

    //月视图切日视图
    function scrollToMonth(year, month) {
        vbar.active = true
        var targetY = 0
        theView.contentY = 0
        for (var i = 0; i < theModel.count; i++) {
            var modelObj = theModel.get(i)
            var token = modelObj.dayToken
            var dates = token.split("-")
            if(year === dates[0] && month === dates[1]) {
                break
            }
            targetY += dayHeights[i]
        }

        // 当日视图滚到底部时，theview的originY会发生改变，将会导致contentY的定位值异常
        // 因此实际滚动值需要通过originY来修正
        theView.contentY = targetY + theView.originY
    }

    function unSelectAll() {
        theView.sigUnSelectAll()
        selectedPaths = []
        GStatus.selectedPaths = []
    }

    function flushModel() {
        //0.清理
        theModel.clear()
        theModel.selectedPathObjs = []
        selectedPaths = []
        dayHeights = []
        //1.获取日期
        var days = []
        if (Number(FileControl.getConfigValue("", "loadDayView", 1)))
            days = albumControl.getDays()

        //2.构建model
        var dayHeight = 0
        var listHeight = 0
        theView.listContentHeight = 0
        var dayPaths
        for (var i = 0;i !== days.length;++i) {
            theModel.append({dayToken: days[i]})

            // 当前日期列表选中数据初始化
            dayPaths = []
            theModel.selectedPathObj = {"id": i, "paths": dayPaths}
            theModel.selectedPathObjs.push(theModel.selectedPathObj)

            // 计算每个日期列表高度
            dayPaths = albumControl.getDayPaths(days[i])
            listHeight = Math.abs(Math.ceil(dayPaths.length / Math.floor(width / realCellWidth)) * realCellWidth)
            dayHeight = timeLineLblHeight + timeLineLblMargin + selAllCheckBoxHeight + listHeight
            dayHeights.push(dayHeight)
            theView.listContentHeight += dayHeight
        }
    }

    function executeScrollBar(delta) {
        if (theView.contentHeight <= theView.height)
            return

        vbar.active = true
        theView.contentY -= delta

        if(vbar.position < 0) {
            vbar.position = 0
        } else if(vbar.position > 1 - theView.height / theView.contentHeight) {
            vbar.position = 1 - theView.height / theView.contentHeight
        }
    }

    // 刷新日视图列表已选路径
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
        if (visible) {
            GStatus.selectedPaths = selectedPaths
        }
    }

    // 日视图的标题栏
    Item {
        id: allCollectionTitleRect
        width: parent.width - GStatus.verticalScrollBarWidth
        height: timeLineLblMargin + timeLineLblHeight + selAllCheckBoxHeight
        z: 2
        Rectangle {
            color: DTK.themeType === ApplicationHelper.LightType ? "#f8f8f8"
                                                                  : "#202020"
            anchors.fill : parent
            opacity: 0.95
        }

        // 标题栏时间标签
        Label {
            id: titleTimeLineLabel
            z:3
            anchors {
                top: parent.top
                topMargin: timeLineLblMargin
                leftMargin: timeLineLblMargin
                left: parent.left
            }
            font: DTK.fontManager.t3
            visible: text !== ""
            text : ""
        }

        //标题栏照片数label
        Label {
            id: titlePhotoNumberLabel
            z:3
            height: selAllCheckBoxHeight
            anchors {
                top: titleTimeLineLabel.bottom
                leftMargin: timeLineLblMargin
                left: parent.left
            }
            topPadding: -1
            text : ""
            visible: text !== ""
        }
    }

    function generatePhotoVideoString(picTotal, videoTotal) {
        var str = "";
        if (picTotal === 1) {
            str += qsTr("1 photo ");
        } else if (picTotal > 1) {
            str += qsTr("%1 photos ").arg(picTotal);
        }

        if (videoTotal === 1) {
            str += qsTr("1 video");
        } else if (videoTotal > 1) {
            str += qsTr("%1 videos").arg(videoTotal);
        }

        return str;
    }

    //dayToken: 日期令牌，用于获取其它数据
    ListModel {
        id: theModel
        property var selectedPathObj: {"id":0, "paths":[]}
        property var selectedPathObjs: []
    }

    ListView {
        id: theView
        clip: true
        model: theModel
        width: parent.width
        height: parent.height
        delegate: theDelegate
        interactive: false

        Rectangle {
            anchors.fill:  parent
            z:-1
            color: DTK.themeType === ApplicationHelper.LightType ? "#f8f8f8"
                                                                  : "#202020"
        }

        //鼠标正在按下状态
        property bool inPress: false
        //框选滚动方向
        property var scrollDirType: Album.Types.NoType
        property var listContentHeight
        property int rectSelScrollOffset: GStatus.rectSelScrollStep

        signal sigUnSelectAll()
        signal dbClicked(string url)

        //激活滚动条
        ScrollBar.vertical: ScrollBar {
            id: vbar
            active: false
            signal topDelegateIndexChanged(int newIndex)

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
                if (firstItemIndex === -1)
                    return
                var dayToken = theModel.get(firstItemIndex).dayToken

                var picTotal = albumControl.getDayInfoCount(dayToken, 3)
                var videoTotal = albumControl.getDayInfoCount(dayToken, 4)

                var str = generatePhotoVideoString(picTotal, videoTotal);

                numLabelTextUpdate(str)
                var dates = dayToken.split("-")
                var timeLineText = qsTr("%1/%2/%3").arg(dates[0]).arg(Number(dates[1])).arg(Number(dates[2]))
                timelineLabelTextUpdate(timeLineText)
            }
        }

        MouseArea {
            // 鼠标区域需要包含空白区域，否则点击空白区域会拖动相册应用
            anchors.fill: parent.contentHeight > parent.height ? parent.contentItem : parent
            acceptedButtons: Qt.LeftButton
            propagateComposedEvents: true

            property bool ctrlPressed: false //记录ctrl是否按下

            id: theMouseArea

            onClicked: (mouse)=> {
                //允许鼠标事件传递给子控件处理,否则鼠标点击缩略图收藏图标不能正常工作
                //同时propagateComposedEvents需设置为true
                //注意：不能传递onPressed、onReleased等基础事件，会有bug；合成事件onClicked等可以传递
                mouse.accepted = false
            }

            onPressed: (mouse)=> {
                if(mouse.button == Qt.RightButton) {
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
                parent.inPress = true
                rubberBand.x1 = mouse.x
                rubberBand.y1 = mouse.y
                rubberBand.x2 = mouse.x
                rubberBand.y2 = mouse.y
                mouse.accepted = true
            }
            onDoubleClicked: (mouse)=> {
                if (GStatus.selectedPaths.length > 0)
                    theView.dbClicked(GStatus.selectedPaths[0])

                parent.inPress = false
                rubberBand.clearRect()

                mouse.accepted = true
            }
            onMouseXChanged: (mouse)=> {
                if(mouse.button === Qt.RightButton) {
                    mouse.accepted = false
                    return
                }

                rubberBand.x2 = mouse.x

                mouse.accepted = true
            }
            onMouseYChanged: (mouse)=> {
                if(mouse.button === Qt.RightButton) {
                    mouse.accepted = false
                    return
                }

                // 刷新矩形第二锚点，内部触发updateRect，保证y2值标记为矩形底部坐标
                rubberBand.y2 = mouse.y

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

                mouse.accepted = true
            }
            onReleased: (mouse)=> {
                if(mouse.button === Qt.RightButton) {
                    mouse.accepted = false
                    return
                }

                parent.inPress = false

                // ctrl按下，鼠标点击事件释放时，需要再发送一次框选改变信号，用来在鼠标释放时实现ctrl取消选中的功能
                if ((Qt.ControlModifier & mouse.modifiers) && rubberBand.width < 3 & rubberBand.height < 3) {
                    rubberBand.rectSelChanged()
                }

                ctrlPressed = false

                theView.scrollDirType = Album.Types.NoType
                rubberBand.clearRect()

                var gPos = theMouseArea.mapToGlobal(mouse.x, mouse.y)
                sigListViewReleased(gPos.x, gPos.y)

                mouse.accepted = true
            }

            onWheel: (wheel)=> {
                var datla = wheel.angleDelta.y / 2
                if (Qt.ControlModifier & wheel.modifiers) {
                    // 按住ctrl，缩放缩略图
                    var curValue = statusBar.sliderValue
                    if (datla > 0)
                        statusBar.setSliderWidgetValue(curValue + 1)
                    else
                        statusBar.setSliderWidgetValue(curValue - 1)
                } else {
                    // 正常滚动显示缩略图内容
                    executeScrollBar(datla)
                }
            }

            //橡皮筋控件
            RubberBand {
                id: rubberBand
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
                        var newY2 = rubberBand.y2 + theView.rectSelScrollOffset
                        if (newY2 <= theView.listContentHeight) {
                            rubberBand.y2 = newY2
                            theView.contentY = theView.contentY + theView.rectSelScrollOffset + theView.originY
                        } else {
                            // 选择框底部最大值为内容区域底部
                            theView.contentY = theView.listContentHeight - theView.height
                            rubberBand.y2 = theView.listContentHeight
                            rectScrollTimer.stop()
                        }
                    } else if (theView.scrollDirType === Album.Types.ToTop) {
                        if (rubberBand._top() < 0) {
                            rectScrollTimer.stop()
                            return
                        }

                        // 矩形顶部向上延展
                        if (theView.contentY <= rubberBand._bottom() || rubberBand._bottom() === rubberBand._top()) {
                            var newTop = rubberBand._top() - theView.rectSelScrollOffset
                            if (newTop > 0) {
                                rubberBand.y2 = newTop
                                theView.contentY = theView.contentY - theView.rectSelScrollOffset + theView.originY
                            } else {
                                // 选择框顶部最小值为内容区域顶部
                                rubberBand.y2 = 0
                                theView.contentY = 0 + theView.originY

                                rectScrollTimer.stop()
                            }
                        } else {
                            // 矩形框底部向上收缩
                            var newBottom = rubberBand._bottom() - theView.rectSelScrollOffset
                            if (newBottom > rubberBand._top()) {
                                rubberBand.y2 = newBottom
                                theView.contentY = theView.contentY - theView.rectSelScrollOffset + theView.originY
                            } else {
                                var srcollOffset = Math.abs(rubberBand.y1 - rubberBand.y2)
                                rubberBand.y2 = rubberBand.y1
                                theView.contentY = theView.contentY - srcollOffset + theView.originY
                            }
                        }
                    }
                }
            }
        }
    }
    Connections {
        target: GStatus
        function onSigPageUp() {
            if (visible) {
                executeScrollBar(scrollDelta)
            }
        }

        function onSigPageDown() {
            if (visible) {
                executeScrollBar(-scrollDelta)
            }
        }
    }

    Component {
        id: theDelegate

        Item {
            id: delegateRect
            width: theView.width
            height: timeLineLblHeight + timeLineLblMargin + selAllCheckBoxHeight + theSubView.height

            property string m_dayToken: dayToken

            Label {
                id: timeLineLabel
                font: DTK.fontManager.t3
                height: timeLineLblHeight
                anchors {
                    top: parent.top
                    topMargin: timeLineLblMargin
                    left: parent.left
                    leftMargin: timeLineLblMargin
                }
            }

            CheckBox {
                id: selectAllBox
                height: selAllCheckBoxHeight
                anchors {
                    top: timeLineLabel.bottom
                    left: timeLineLabel.left
                }
                checked: theSubView.haveSelectAll
                visible: selectedPaths.length > 0
                onClicked: {
                    if(checked) {
                        theSubView.selectAll(true)
                    } else {
                        theSubView.selectAll(false)
                    }
                }
            }

            Connections {
                target: dayView
                function onSigListViewPressed(x, y) {
                    var object = selectAllBox.mapFromGlobal(x,y)
                    if (selectAllBox.contains(object)) {
                        checkBoxClicked = true
                        if (selectAllBox.checkState === Qt.Checked) {
                            selectAllBox.checkState = Qt.Unchecked
                            theSubView.selectAll(false)
                        } else {
                            selectAllBox.checkState = Qt.Checked
                            theSubView.selectAll(true)
                        }
                    }
                }

                function onSigListViewReleased(x, y) {
                    checkBoxClicked = false
                }
            }

            Label {
                id: numLabelTitle
                height: selAllCheckBoxHeight
                anchors {
                    top: timeLineLabel.bottom
                    left: selectAllBox.visible ? selectAllBox.right : timeLineLabel.left
                }
                topPadding: -1
            }

            ThumbnailListViewAlbum {
                id: theSubView
                anchors {
                    top: selectAllBox.bottom
                    left: selectAllBox.left
                }
                enableMouse: false
                width: parent.width
                height: Math.abs(Math.ceil(theSubView.count / Math.floor((parent.width) / itemWidth)) * itemHeight)
                thumnailListType: Album.Types.ThumbnailDate
                proxyModel.sourceModel: Album.ImageDataModel { id: dataModel; modelType: Album.Types.DayCollecttion}

                Connections {
                    target: rubberBand
                    function onRectSelChanged() {
                        var pos1 = theMouseArea.mapToItem(theSubView, rubberBand._left(), rubberBand._top())
                        var pos2 = theMouseArea.mapToItem(theSubView, rubberBand._right(), rubberBand._bottom())
                        var rectsel = albumControl.rect(pos1, pos2)
                        var rectList = Qt.rect(0, 0, theSubView.width, theSubView.height)
                        var rect = albumControl.intersected(rectList, rectsel)
                        var bDetectMousePrees = albumControl.manhattanLength(pos1, pos2) < 3 // 识别此次框选事件是否为鼠标点击事件，以便在列表控件处理ctrl按键相关的操作
                        theSubView.flushRectSel(rect.x, rect.y, rect.width, rect.height, theMouseArea.ctrlPressed, bDetectMousePrees, theView.inPress)
                    }
                }

                // 监听缩略图子控件选中状态，一旦改变，更新日视图所有选中路径
                Connections {
                    target: theSubView
                    function onSelectedChanged() {
                        if (index > -1) {
                            theModel.selectedPathObjs[index].paths = theSubView.selectedUrls
                        }
                        updateSelectedPaths()
                    }
                }

                Connections {
                    target: theView
                    function onSigUnSelectAll() {
                        theSubView.selectAll(false)
                    }
                }

                Connections {
                    target: theView
                    function onDbClicked(url) {
                        var openPaths = theSubView.allUrls()
                        if (openPaths.indexOf(url) !== -1) {
                            var pos = theMouseArea.mapToItem(theSubView, rubberBand._left(), rubberBand._top())
                            theSubView.viewImageFromOuterDbClick(pos.x, pos.y)
                        }
                    }
                }
            }

            function flushView() {
                var picTotal = albumControl.getDayInfoCount(m_dayToken, 3)
                var videoTotal = albumControl.getDayInfoCount(m_dayToken, 4)
                //1.刷新图片显示
                dataModel.dayToken = m_dayToken
                theSubView.proxyModel.refresh()

                //2.刷新checkbox
                var str = generatePhotoVideoString(picTotal, videoTotal);
                numLabelTitle.text = str

                //3.刷新标题
                var dates = m_dayToken.split("-")
                timeLineLabel.text = qsTr("%1/%2/%3").arg(dates[0]).arg(Number(dates[1])).arg(Number(dates[2]))

                if (isFirstLoad) {
                    isFirstLoad = false
                    timelineLabelTextUpdate(timeLineLabel.text)
                    numLabelTextUpdate(numLabelTitle.text)
                }
            }

            Component.onCompleted: {
                flushView()
            }
        }
    }

    onVisibleChanged: {
        // 窗口显示时，重置显示内容
        if (visible && !GStatus.backingToMainAlbumView) {
            //清除选中状态
            unSelectAll()
            flushModel()
        }
    }

    Component.onCompleted: {
        dayView.timelineLabelTextUpdate.connect(setTimeLine)
        dayView.numLabelTextUpdate.connect(setNumLabel)
        GStatus.sigFlushAllCollectionView.connect(flushModel)
    }
}
