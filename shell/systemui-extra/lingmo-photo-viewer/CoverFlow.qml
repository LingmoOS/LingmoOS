import QtQuick 2.12
import QtQuick.Controls 2.12
import "./part"
Rectangle {
    id: coverflow
    anchors.fill: parent
    property ListModel imagemodel
    property int itemCount: variables.imagelist.length >= 5? 5 : variables.imagelist.length != 0 ? variables.imagelist.length : 1
    property int itemMargin: 40 //item间距
    property int itemDistance: coverflow.width + itemMargin //item+间距
    color: "transparent"
    signal changeImageFromView(var path,var changeWay) //切图
    property int backItem: -1 //前一个item的index
    property bool stopMove: true //控制pathvie能否滑动
    property string moveWay: "default"


    PathView {
        anchors.fill: parent
        objectName: "pathWid"
        id: pathView
        model: imagemodel
        movementDirection: moveWay == "default"? PathView.Shortest : moveWay == "back" ? PathView.Negative : PathView.Positive
        flickDeceleration:1000//轻弹减速的速率
        snapMode: PathView.SnapOneItem
        interactive: !stopMove ? false : itemCount <= 1 || variables.navigatorState ?  false : true
        delegate: Item {
            id: delegateItem
            width: coverflow.width
            height: coverflow.height
            clip: true
            //主要的图片区域
            MainImageArea {
                objectName: "imageItem"
                id: imageArea
                x: 0
                y: 0
                width: mainWindow.width
                height: mainWindow.height
                imageUrl: url
                imageType: type
                itemIndex: index
            }
        }
        path: variables.imagelist.length >= 5? coverFlowPath : variables.imagelist.length == 4 ? coverFlowPathFourth : variables.imagelist.length == 3 ? coverFlowPathThird :variables.imagelist.length >= 2 ? coverFlowPathSecond :  coverFlowPathFirst
        pathItemCount: itemCount
        highlightRangeMode: PathView.StrictlyEnforceRange
        preferredHighlightBegin: 0.5
        preferredHighlightEnd: 0.5

        onMovementEnded: {
            //切图
            if (variables.imagelist.length - 1 >= pathView.currentIndex) {
                if (backItem != pathView.currentIndex) {
                    if (pathView.currentIndex - backItem > 0) {
                        if (backItem == 0 && pathView.currentIndex == pathView.count - 1) {
                            changeImageFromView(variables.imagelist[pathView.currentIndex],false)
                        } else {
                            changeImageFromView(variables.imagelist[pathView.currentIndex],true)
                        }
                    } else if (pathView.currentIndex - backItem < 0) {
                        if (pathView.currentIndex == 0 && backItem == pathView.count - 1) {
                            changeImageFromView(variables.imagelist[pathView.currentIndex],true)
                        } else {
                            changeImageFromView(variables.imagelist[pathView.currentIndex],false)
                        }
                    }
                    backItem = pathView.currentIndex
                }
            } else {
                console.log("切换图片失败，当前view和图片列表不匹配")
            }
            //将动图的显示进行重置-用以规避动图的显示过程可以被看到的问题
            for (var i = 0; i < pathView.count; i++) {
                if (i !== pathView.currentIndex) {
                    if (variables.imageListEveryAddType[i]) {
                        if (model.get(i).url !== "qrc:/res/res/loadgif.gif") {
                            model.setProperty(i,"url","qrc:/res/res/loadgif.gif")
                        }
                    }
                }
            }
        }

        Component.onCompleted: {
            backItem = pathView.currentIndex
        }
    }
    //item的行进路线-pathLine
    Path {
        id:coverFlowPath
        startX: -2*itemDistance - itemMargin / 2
        startY: coverflow.height / 2
        PathLine {x:-itemDistance - itemMargin / 2; y:coverflow.height / 2 }
        PathLine {x:-itemMargin / 2; y:coverflow.height / 2 }
        PathLine {x:-itemMargin / 2 + itemDistance; y:coverflow.height / 2 }
        PathLine {x:-itemMargin / 2 + 2 * itemDistance; y:coverflow.height / 2 }
        PathLine {x:-itemMargin / 2 + 3 * itemDistance; y:coverflow.height / 2 }
    }

    Path{
        id: coverFlowPathFourth
        startX: -itemDistance / 2 - itemMargin / 2 - itemDistance
        startY: coverflow.height / 2
        PathLine { x: -itemDistance / 2 - itemMargin / 2 +itemDistance- itemDistance ; y:coverflow.height / 2 }
        PathLine { x: -itemDistance / 2 - itemMargin / 2 + 2*itemDistance- itemDistance ; y:coverflow.height / 2 }
        PathLine { x: -itemDistance / 2 - itemMargin / 2 + 3*itemDistance- itemDistance; y:coverflow.height / 2 }
        PathLine { x: -itemDistance / 2 - itemMargin / 2 + 4*itemDistance- itemDistance ; y:coverflow.height / 2 }
    }

    Path{
        id: coverFlowPathThird
        startX: -itemDistance - itemMargin / 2
        startY: coverflow.height / 2
        PathLine { x: -itemMargin / 2; y:coverflow.height / 2 }
        PathLine { x: -itemMargin / 2 + itemDistance; y:coverflow.height / 2 }
        PathLine { x: -itemMargin / 2 + 2 * itemDistance; y:coverflow.height / 2 }

    }
    Path{
        id: coverFlowPathSecond
        startX: -itemDistance / 2 - itemMargin / 2
        startY: coverflow.height / 2
        PathLine { x: -itemDistance / 2 - itemMargin / 2 +itemDistance ; y:coverflow.height / 2 }
        PathLine { x: -itemDistance / 2 - itemMargin / 2 + 2*itemDistance; y:coverflow.height / 2 }
    }
    Path{
        id: coverFlowPathFirst
        startX: 0
        startY: coverflow.height/2
        PathLine { x: coverflow.width; y:coverflow.height / 2 }
    }
    //通知pathview更换图片
    function noticeChangeIndex(changeIndex) {
        if ((pathView.currentIndex === changeIndex) && (itemCount > 1)) {
            return
        }
        pathView.currentIndex = changeIndex
        backItem = pathView.currentIndex
    }

}
