import QtQuick 2.12
import QtQuick.Window 2.0
import QtQuick.Controls 2.12
import "./part"
import "./mousebase"
import "./signcanvas"

Rectangle {
    objectName: "mainRect"
    id: mainWindow
    visible: true
    property string dropImagePath: ""
    signal sendDropImagePath(var path)
    property int mouseMoveDistance: 0
    property point startPressPoint: Qt.point(-1,-1)
    property point releasePoint: Qt.point(-1,-1)
    property bool mouseIsDoubleClick: false
    signal changeImage(var nextOrBack)
    signal doubleSignal()
    signal restoreOrig()
    property int signListLength: variables.operateTime
    signal mousePressOrRelease()
    property bool startClick: true
    signal copyShortCuts()
    //19807-signal
    signal noticeViewChangeIndex(var acturalIndex)
    signal changeImageFromView(var path,var changeWay)
    Shortcut {
        sequence: StandardKey.Copy
        onActivated: {
            copyShortCuts()
        }
    }
    Timer {
        id: clickTimer
        interval: 200
        onTriggered: {
            mousePressOrRelease()
        }
    }

    //鼠标事件响应区域
    MouseAreaEvent { id: mouseshortcuts }
    //外边框---最外层的，需要用到别的组件相对于它的位置
    Image {
        id: bgimage
        anchors.fill: parent
        MouseArea {
            id: mousemouse
            enabled: !variables.signMouseStart
            anchors.fill: parent
            hoverEnabled: true
            onPositionChanged: {
                variables.mousePos = Qt.point(mouseX, mouseY)
            }

            onDoubleClicked: {
                clickTimer.stop()
                mouseIsDoubleClick = true
                doubleSignal()
                restoreOrig()
            }

            onClicked: {
                if (!clickTimer.running) {
                    clickTimer.start()
                } else {
                    return
                }
            }
        }
    }
    ListModel{
        id:model
    }
    CoverFlow{
        id:coverFLow
        objectName: "coverWid"
        anchors.fill:parent
        imagemodel:model
    }
    Variables {
        id: variables
        onOperateTimeChanged: {
            signListLength = variables.operateTime
        }
        onRestoreOrigToToolbarChanged: {
            //告诉c++前端需要使图片自适应窗口
            restoreOrig()
        }
    }

    property ListModel model
    ImageBaseFunction {id: imageBase}
    SignBase {id: signBase}

    onWidthChanged: {
        variables.curWidW = width + 1
    }
    onHeightChanged: {
        variables.curWidH = height + 1
    }
    
    //拖拽文件打开图片
    DropArea {
        anchors.fill: parent
        onDropped: {
            if (variables.operateImageWay == 15 || variables.operateImageWay == 9 || variables.operateImageWay == 10) {
                return
            }
            if (drop.hasUrls) {
                if (drop.urls.length > 0) {
                    dropImagePath = drop.urls[0]
                    sendDropImagePath(drop.urls[0])
                }
            }
        }
    }
    Component.onCompleted: {
        mainWindow.noticeViewChangeIndex.connect(coverFLow.noticeChangeIndex)
        coverFLow.changeImageFromView.connect(mainWindow.changeImageFromView)
    }

    function setOperateWay(way) {

        variables.operateImageWay = way
        if (variables.operateImageWay == 15) {
            coverFLow.stopMove = false
        } else {
            coverFLow.stopMove = true
        }
        if (variables.operateImageWay == 17) {
            coverFLow.moveWay = "next"
        } else if (variables.operateImageWay == 18) {
            coverFLow.moveWay = "back"
        } else {
            coverFLow.moveWay = "default"
        }
    }

    function setFormat(format) {
        variables.format = format
    }
    //sign
    function setpainterType(type) {
        variables.painterType = type
    }
    function setpainterThickness(thickness) {
        variables.painterThickness = thickness
    }
    function setpainterColor(color) {
        variables.painterColor = color
    }
    function setBold(isBold) {
        variables.isBold = isBold
    }
    function setDeleteLine(isDeleteLine) {
        variables.isDeleteLine = isDeleteLine
    }
    function setUnderLine(isUnderLine) {
        variables.isUnderLine = isUnderLine
    }
    function setItalics(isItalics) {
        variables.isItalics = isItalics
    }
    function setFontType(fontType) {
        variables.fontType = fontType
    }
    function setFontSize(fontSize) {
        variables.fontSize = fontSize
    }
    function getOperateTime() {
        if (variables.objList.length > 0) {
            if ( variables.objList[variables.objList.length - 1].objectName == "textEditPage") {
                variables.objList[variables.objList.length - 1].sendTextOperate()
            }
        }
        return variables.operateTime
    }
    function exitOperate() {
        coverFLow.stopMove = true
        variables.needExitPaint = !variables.needExitPaint
    }
    //story 19807-记录当前路径
    function setCurrentPath(currentPath,currentType) {
        variables.currentPath = currentPath
        variables.currentType = currentType
    }

    //通过variables的变量来从父对象的子控件里的----子控件的事件响应
    function imageRestoreOrig() {
        variables.needRestoreOrig = !variables.needRestoreOrig
    }

    //每次需要加载的图片
    function updateLoadImageList(list) {
        variables.loadImageList = list

    }
    //当前图片的位置
    function updateCurrentImageIndex(index) {
        variables.currentImageIndex = index
    }
    //需要删掉的图片路径
    function updateImageDelPath(delPath) {
        variables.imageDelPath = delPath
    }
    //记录当前展示的5个item的所有index，用来加载图片用，与loadImageList是一一对应的
    function updateShowImageIndex(showIndex) {
        variables.showImageIndex = showIndex
    }
    //记录图片展示列表当前是否是动静图
    function updateImageType(imageTypeList) {
        variables.imageTypeMovieOrNormal = imageTypeList

    }

    function setThemeIcon(icon) {
        variables.imageDefault = icon
    }

    //所有图片的list，确定item的数量，加载或预加载图片
    function updateImageList(imageList,imageTypeList,nullImage,tiffOperate) {
        //记录tiff多页图片是否被操作过
        variables.imageHasChanged = tiffOperate
        //确定图片操作的数目
        variables.addImageNum = imageList.length - variables.imagelist.length
        variables.imageTotalNum = imageList.length

        variables.imageListEveryAddType = imageTypeList
        /*===步骤一：确定当前需要展示的item的数量===*/
        createView(imageList)
        /*===步骤二：更新图片列表,此时可以更新图片列表===*/
        variables.imagelist = imageList
        /*===步骤三：将当前的图片加载出来===*/
        loadImage(nullImage)
        /*===步骤四：预加载前后左右的图片===*/
        preloadImage()
    }
    //步骤一：创建或更新model/view
    function createView(imageList) {
        /*判断当前一张图片也没有打开,属于第一次或删除了所有的图片后再一次加载图片*/
        if (variables.imagelist.length == 0 && imageList.length != 0) {
            if (variables.imageListEveryAddType.length != variables.imageTotalNum) {
                console.log("create1 - type 和 path 无法匹配")
                return
            }
            //直接new出整个初始的视图item列表
            for (var i = 0; i < variables.imageTotalNum; i++) {
                model.append({url:variables.imageDefault,type:variables.imageListEveryAddType[i]})
            }
            /*第二次以上打开图片*/
        } else if (variables.imagelist.length > 0) {
            //***先删后加:1.先确定当前list里有图片
            if (variables.imageDelPath != "") {
                //从model里移除
                if (variables.imagelist.length != 0) {
                    for (var j = 0; j < variables.imagelist.length; j++) {
                        if (variables.imageDelPath === variables.imagelist[j]) {
                            model.remove(j)
                        }
                    }
                }
                //从加载过的图片列表里移除
                if (variables.loadedImageList.length != 0) {
                    for (var n = variables.loadedImageList.length - 1; n >= 0; n--) {
                        if (variables.imageDelPath === variables.loadedImageList[n]) {
                            variables.loadedImageList.splice(n,1)
                        }
                    }
                }
            }
            //***2.当前是否有需要添加的
            if (variables.addImageNum > 0) {
                if (variables.imageListEveryAddType.length != variables.imagelist.length - variables.imageDelPath.length + variables.addImageNum) {
                    console.log("create2 - type 和 path 无法匹配")
                    return
                }
                //model需要创建num个item,这里从0开始因为，后端是添加的会增加在顶端
                for (var m = 0; m < variables.addImageNum; m++) {
                    //初始化imageTotalNum个item
                    model.insert(m,{url:variables.imageDefault,type:variables.imageListEveryAddType[m]})
                }
            }
            /*其他情况只需要判断当前的index是不是正确的，是否只为切换图片*/
        } else {
            //确定当前的index
            if (variables.currentImageIndex != -1) {
                noticeViewChangeIndex(variables.currentImageIndex)
            } else {
                console.log("当前无需加载图片")
            }
        }
    }
    //步骤三
    function loadImage(nullImage) {
        //加载当前图片
        updateViewImageUrl(nullImage)
        //通知pathview换图
        noticeViewChangeIndex(variables.currentImageIndex)
    }

    //步骤三中，加载当前图片
    function updateViewImageUrl(nullImage) {

        if (variables.imageTypeMovieOrNormal.length <= 0) {
            console.log("预加载类型列表异常")
            return
        }
        if (variables.showImageIndex.length != variables.imageTypeMovieOrNormal.length) {
            console.log("预加载图片类型列表和index不匹配")
            return
        }

        var currentType = 0;
        //获得当前的type
        currentType = variables.imageListEveryAddType[variables.currentImageIndex]
        //判断当前的type是否时动图的type
        if (currentType) {
            if (model.get(variables.currentImageIndex).url !== "file://"  + variables.imagelist[variables.currentImageIndex] ) {
                model.setProperty(variables.currentImageIndex, "url", "file://"  + variables.imagelist[variables.currentImageIndex])
            }
        } else {
            if (variables.currentType != "multiTiff") {
                model.setProperty(variables.currentImageIndex, "url", "image://main/" + variables.imagelist[variables.currentImageIndex])
            }

        }
    }

    //步骤四：预加载其他几张图片
    function preloadImage() {
        if (variables.loadImageList.length <= 0) {
            console.log("无需预加载图片或预加载列表异常")
            return
        }
        if (variables.imageTypeMovieOrNormal.length < 0) {
            console.log("预加载类型列表异常")
            return
        }

        if (variables.loadImageList.length != variables.showImageIndex.length) {
            console.log("预加载图片列表和index不匹配")
            return
        }
        if (variables.imageTypeMovieOrNormal.length != variables.loadImageList.length ) {
            console.log("预加载图片列表和type不匹配")
            return
        }

        var type = 0;
        var realLoadImageList = arrCopy(variables.loadImageList)
        var realLoadImageType = arrCopy(variables.imageTypeMovieOrNormal)
        var realLoadeImageIndex = arrCopy(variables.showImageIndex)

        for (var i = 0; i < variables.loadImageList.length; i++) {
            //无需加载当前图片
            for (var p = 0; p < variables.loadedImageList.length; p++) {
                if (variables.loadedImageList[p] === variables.loadImageList[i]) {
                    realLoadImageList.splice(realLoadImageList.indexOf(variables.loadImageList[i]),1)
                    realLoadImageType.splice(realLoadImageType.indexOf(variables.imageTypeMovieOrNormal[i]),1)
                    realLoadeImageIndex.splice(realLoadeImageIndex.indexOf(variables.showImageIndex[i]),1)
                }
            }
        }

        for (var j = 0; j < variables.showImageIndex.length; j++) {
            if (variables.showImageIndex[j] != variables.currentImageIndex) {
                var hasLoaded = false;
                for (var n = 0; n < variables.loadedImageList.length;n++) {
                    if (variables.loadedImageList[n] == variables.imagelist[variables.showImageIndex[j]]) {
                        hasLoaded = true
                    }
                }
                if (hasLoaded) {
                    continue
                }
                if (variables.imageListEveryAddType[variables.showImageIndex[j]]) {
                    model.setProperty(variables.showImageIndex[j],"url","qrc:/res/res/loadgif.gif")
                } else {
                    var sUrl = model.get(variables.showImageIndex[j]).url
                    if (sUrl.includes(variables.imagelist[variables.showImageIndex[j]]))
                    {
                        model.setProperty(variables.showImageIndex[j],"url", "")
                    }
                    model.setProperty(variables.showImageIndex[j],"url","image://thumb/" + variables.imagelist[variables.showImageIndex[j]])
                }
            }
        }
        for (var k = 0; k < realLoadImageList.length;k++) {
            if (realLoadImageList[k] != variables.currentImageIndex) {
                var suffix = realLoadImageList[k].substring(realLoadImageList[k].lastIndexOf(".")+1,
                                                            realLoadImageList[k].length);
                //更新加载图片列表
                variables.loadedImageList.push(variables.imagelist[realLoadeImageIndex[k]])
            }
        }

    }
    function arrCopy(arr) {
        var tempArr = []

        if (arr.length == 0) {
            return tempArr
        }
        for (var i = 0; i < arr.length; i++) {
            tempArr[tempArr.length] = arr[i]
        }
        return tempArr
    }

    function reloadImage() {
        if (variables.loadedImageList.length != 0) {
            for (var n = variables.loadedImageList.length - 1; n >= 0; n--) {
                if (variables.imagelist[variables.currentImageIndex] !== variables.loadedImageList[n]) {
                    variables.loadedImageList.splice(n,1)
                }
            }
        }
        preloadImage()
    }
}

