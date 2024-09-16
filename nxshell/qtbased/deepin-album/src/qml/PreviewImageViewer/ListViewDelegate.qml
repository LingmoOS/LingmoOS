// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.11
import QtGraphicalEffects 1.0
import org.deepin.dtk 1.0

Item {
    id: container
    height: container.ListView.view.height - 40
    width: 30
    y: 20

    property int imgRadius: 3
    // 当前缩略图索引的图片路径
    property var currentSource: imageViewer.sourcePaths[index]
    // 判断是否为多页图
    property bool isMultiImage: FileControl.isMultiImage(currentSource)
    // 判断图片是否存在
    property bool isImageExist: FileControl.imageIsExist(currentSource)

    Rectangle {
        id: enterShader
        height: parent.height + (2 * imgRadius)
        width: parent.width + (2 * imgRadius)

        anchors {
            top: parent.top
            topMargin: -imgRadius
            left: parent.left
            leftMargin: -imgRadius
        }

        radius: imgRadius * 2

        color: "transparent"
        border.color: "#0081FF"
        border.width: imgRadius
        visible: false
    }

    // 图片保存完成，缩略图区域重新加载当前图片
    Connections {
        target: FileControl

        // 图片被移动、替换、删除时触发
        // imageFileChanged(const QString &filePath, bool isMultiImage = false, bool isExist = false);
        onImageFileChanged: {
            // 变更的图片可能非当前展示图片，但同样需要更新
            if (filePath === container.currentSource) {
                // 为多页图时根据isImageExist自动切换状态
                container.currentSource = ""
                container.currentSource = filePath

                img.source = ""
                img.source = FileControl.isSvgImage(filePath) ? filePath : "image://ThumbnailImage/" + filePath

                // 重新加载，复位旋转状态
                container.rotation = 0
            }
        }
    }

    Item {
        id: imgItem
        width: container.width - 10
        height: container.height - 10
        anchors.fill: parent
        // 首张图片为多页图时，图片状态在 multiImageLoader.onLoaded 变更，需要设置默认状态，states 变更后恢复
        visible: true

        Image {
            id: img
            width: container.width - 10
            height: container.height - 10
            anchors.centerIn: parent

            smooth: false
            anchors.fill: parent
            source: FileControl.isSvgImage(currentSource) ? currentSource : "image://ThumbnailImage/" + currentSource
            sourceSize.width: 100
            sourceSize.height: 100
            asynchronous: true
            visible: false
            cache: false

            onStatusChanged: {
                // 错误图片显示撕裂图
                if (img.status === Image.Error) {
                    img.source = "qrc:/res/picture_damaged_58.svg"
                }
            }
        }

        Rectangle {
            id: maskRect
            anchors.fill: img
            visible: false
            radius: imgRadius
        }

        OpacityMask {
            id: imgMask
            anchors.fill: img
            source: img
            maskSource: maskRect
        }
    }

    // 多页图展开处理，加载组件 MultiImageListView
    Loader {
        id: multiImageLoader
        width: container.width
        height: container.height
        asynchronous: true

        onLoaded: {
            multiImageLoader.item.source = currentSource
            // 绑定多页图帧号变更信号
            indexBinder.target = multiImageLoader.item
            // 由于延迟加载，在之前进行布局时没有采用最终的大小，加载完成后，重新调整当前项位置
            container.ListView.view.positionViewAtIndex(index, ListView.Center)

            // 手动控制隐藏单页缩略图，防止缩放过程中显示白色背景
            imgItem.visible = false
        }

        Connections {
            id: frameSigConn
            target: multiImageLoader.item
            // MultiImageListView 发送索引切换信号, 传入点击切换的帧索引 switchIndex
            onSwitchFrameIndex: {
                if (imageViewer.currentIsMultiImage) {
                    imageViewer.frameIndex = switchIndex
                }
            }
        }

        // 绑定 imageViewer 的多页图帧号变更到多页缩略图列表组件 MultiImageListView
        Binding {
            id: indexBinder
            property: "frameIndex"
            value: imageViewer.frameIndex
        }
    }

    // 图片数角标
    Loader {
        id: anchorLoader
        height: 14
        width: Math.max(20, implicitWidth)
        anchors {
            right: parent.right
            bottom: parent.bottom
        }

        // 非多页图无需实例化
        active: isMultiImage && isImageExist
        // 仅多页图显示角标(为焦点时不加载)
        visible: isMultiImage && isImageExist

        sourceComponent: Rectangle {
            id: anchorRect
            anchors.fill: parent
            radius: 4

            // 多页图图片数角标
            Label {
                id: anchorLabel
                anchors.fill: parent
                z: DTK.AboveOrder

                topPadding: 3
                bottomPadding: 3
                leftPadding: 2
                rightPadding: 2
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                font.weight: Font.DemiBold
                font.pixelSize: 11
                text: {
                    // 取得当前索引的图片帧号
                    var count = FileControl.getImageCount(currentSource)
                    return (count <= 999) ? count : "999+"
                }

                background: Rectangle {
                    implicitHeight: 14
                    implicitWidth: 14
                    radius: 4

                    gradient: Gradient {
                        GradientStop {
                            position: 0.0
                            color: "#FFC3C3C3"
                        }
                        GradientStop {
                            position: 1.0
                            color: "#FFD8D8D8"
                        }
                    }
                }
            }

            // 图片角标的内阴影
            InnerShadow {
                anchors.fill: anchorLabel
                verticalOffset: -1
                color: Qt.rgba(0, 0, 0, 0.1)
                source: anchorLabel
            }

            // 图片角标的外阴影
            DropShadow {
                anchors.fill: anchorLabel
                verticalOffset: 1
                cached: true
                radius: 2
                samples: 4
                color: Qt.rgba(0, 0, 0, 0.3)
                source: anchorLabel
            }
        }
    }

    MouseArea {
        id: mouseArea
        // 当前项不使用，多页图时，需要将点击事件穿透到下层
        enabled: index !== container.ListView.view.currentIndex
        anchors.fill: parent
        hoverEnabled: true

        onClicked: {
            container.ListView.view.currentIndex = index
            container.forceActiveFocus()
            source = currentSource
            imageViewer.index = index
            // 鼠标点击时固定多页图帧号为0
            imageViewer.frameIndex = 0
        }
    }

    onStateChanged: {
        // 此处状态和 multiImageLoader.onLoaded 关联，手动控制恢复显示状态
        if ("multiPage" != container.state) {
            imgItem.visible = true
        }
    }

    states: [
        // 激活状态
        State {
            name: "active"
            when: container.ListView.view.currentIndex === index && !isMultiImage

            PropertyChanges {
                target: container
                scale: 1.25
                width: height
            }

            PropertyChanges {
                target: enterShader
                visible: true
                z: 1
            }
        },
        // 多页图状态(图片必须存在)
        State {
            name: "multiPage"
            when: container.ListView.view.currentIndex === index && isMultiImage && isImageExist

            PropertyChanges {
                target: enterShader
                visible: true
                z: 1
            }

            PropertyChanges {
                target: anchorLoader
                visible: false
            }

            PropertyChanges {
                target: container
                y: 15
                height: (container.ListView.view.height - 40) * 1.25
                width: {
                    // 最少需要保留两张图片显示的大小
                    var minWidth = 30 + 11
                    // 计算允许的多页图显示宽度，宽度计算以当前界面窗口的宽度计算，此处取宽度值
                    var enableWidth = thumbnailViewBackGround.avaliableListViewWidth - (30 * 2) - 20
                    enableWidth = Math.max(enableWidth, minWidth)

                    // 每张子图片最多占用30px，间隔1px
                    var curMultiImageWidth = (31 * FileControl.getImageCount(currentSource)) - 1
                    return Math.min(619, Math.min(curMultiImageWidth, enableWidth))
                }
                imgRadius: 4
            }

            PropertyChanges {
                target: multiImageLoader
                source: FileControl.isMultiImage(currentSource) ? "qrc:/qml/PreviewImageViewer/MultiImageListView.qml" : ""
            }
        }
    ]

    transitions: Transition {
        reversible: true

        NumberAnimation {
            properties: "scale, x, width, height"
            // 调整不同宽度下的动画时间，最多310ms
            duration: width < 200 ? 100 : width / 2
            easing.type: Easing.OutInQuad
        }
    }
}
