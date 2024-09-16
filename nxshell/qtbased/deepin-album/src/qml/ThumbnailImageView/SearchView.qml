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

import "../Control"
import "../Control/ListView"

BaseView {

    property string currentKeyword: ""
    property var searchResults: new Array
    property string numLabelText: ""
    property string selectedText: getSelectedText(selectedPaths)
    property alias selectedPaths: view.selectedUrls

    onVisibleChanged: {
        if (visible) {
            GStatus.statusBarNumText = searchResultLabel.text
        }
    }

    function searchFromKeyword(UID, keyword) {

        currentKeyword = keyword

        //1.调起C++，执行搜索，将结果刷入UI
        dataModel.albumId = UID
        dataModel.keyWord = keyword
        view.proxyModel.refresh()

        searchResults = view.allUrls()

        getStatusBarText()
    }

    function getStatusBarText() {
        var haveVideo = FileControl.haveVideo(searchResults)
        var havePicture = FileControl.haveImage(searchResults)

        if(havePicture && !haveVideo) {
            if(searchResults.length === 1) {
                searchResultLabel.text = qsTr("1 photo found")
            } else {
                searchResultLabel.text = qsTr("%1 photos found").arg(searchResults.length)
            }
        } else if(haveVideo && !havePicture) {
            if(searchResults.length === 1) {
                searchResultLabel.text = qsTr("1 video found")
            } else {
                searchResultLabel.text = qsTr("%1 videos found").arg(searchResults.length)
            }
        } else if (searchResults.length === 0) {
            searchResultLabel.text = ""
        } else {
            searchResultLabel.text = qsTr("%1 items found").arg(searchResults.length)
        }


        numLabelText = searchResultLabel.text

        if (visible) {
            GStatus.statusBarNumText = numLabelText
        }

        GStatus.selectedPaths = []
    }

    // 刷新选中项目标签内容
    function getSelectedText(paths) {
        var selectedNumText = GStatus.getSelectedNumText(paths, numLabelText)

        if (visible)
            GStatus.statusBarNumText = selectedNumText
        return selectedNumText
    }

    //有结果展示
    Item {
        id: resultViewTitleRect

        width: parent.width - GStatus.verticalScrollBarWidth
        height: GStatus.thumbnailViewTitleHieght

        //搜索标题
        Label {
            id: searchTitle
            text: qsTr("Search results")
            anchors {
                top: parent.top
                topMargin: 12
                left: parent.left
            }
            font: DTK.fontManager.t3
        }

        //幻灯片放映按钮
        RecommandButton {
            id: sliderPlayButton
            visible: searchResults.length > 0
            text: qsTr("Slide Show")
            icon {
                name:"Combined_Shape"
                width: 20
                height: 20
            }
            enabled: FileControl.haveImage(searchResults)
            anchors {
                top: searchTitle.bottom
                left: searchTitle.left
                topMargin: 3
            }
            onClicked: {
                stackControl.startMainSliderShow(searchResults, 0)
            }
        }

        //搜索结果label
        Label {
            id: searchResultLabel
            visible: searchResults.length > 0
            anchors {
                left: sliderPlayButton.right
                leftMargin: 10
                verticalCenter: sliderPlayButton.verticalCenter
            }
            font: DTK.fontManager.t6
        }

        MouseArea {
            anchors.fill: parent
            onPressed: (mouse)=> {
                view.selectAll(false)
                mouse.accepted = false
            }
        }
    }

    //缩略图视图
    ThumbnailListViewAlbum {
        id: view
        anchors {
            top: resultViewTitleRect.bottom
            topMargin: 10
        }
        width: parent.width
        height: parent.height - resultViewTitleRect.height - m_topMargin

        proxyModel.sourceModel: Album.ImageDataModel { id: dataModel; modelType: Album.Types.SearchResult}

        visible: numLabelText !== ""
        property int m_topMargin: 10

        // 监听缩略图列表选中状态，一旦改变，更新globalVar所有选中路径
        Connections {
            target: view
            function onSelectedChanged() {
                if (parent.visible)
                    GStatus.selectedPaths = view.selectedUrls
            }
        }
    }

    //无结果展示
    Item {
        id: noResultView
        visible: searchResults.length === 0
        anchors {
            top: parent.top
            topMargin: searchTitle.height
            left: parent.left
            bottom: parent.bottom
            right: parent.right
        }

        Label {
            id: noResultText
            text: qsTr("No search results")
            font: DTK.fontManager.t4
            anchors.centerIn: parent
            color: Qt.rgba(85/255, 85/255, 85/255, 0.4)
        }
    }

    NumberAnimation {
        id: showAnimation
        target: view
        property: "anchors.topMargin"
        from: 10 + view.height
        to: 10
        duration: GStatus.animationDuration
        easing.type: Easing.OutExpo
    }

    onShowChanged: {
        if (show)
            showAnimation.start()
    }
    Component.onCompleted: {
        GStatus.sigRunSearch.connect(searchFromKeyword)
    }
}
