// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import org.deepin.image.viewer 1.0 as IV

import "../Utils"

// 使用嵌套的ListView进行浏览
BaseImageDelegate {
    id: multiImageDelegate

    ListView {
        id: multiImageView

        width: multiImageDelegate.width
        height: multiImageDelegate.height
        cacheBuffer: 200
        boundsMovement: Flickable.FollowBoundsBehavior
        boundsBehavior: Flickable.StopAtBounds
        highlightRangeMode: ListView.StrictlyEnforceRange
        highlightMoveDuration: 0
        preferredHighlightBegin: 0
        preferredHighlightEnd: 0
        orientation: ListView.Horizontal
        snapMode: ListView.SnapOneItem
        flickDeceleration: 500
        currentIndex: {
            if (isCurrentImage) {
                return IV.GControl.currentFrameIndex
            } else if (index === IV.GControl.currentIndex + 1) {
                return 0
            } else if (index === IV.GControl.currentIndex - 1) {
                return count - 1
            }

            return 0
        }

        // 当处理双击缩放界面时，由于坐标变更，可能误触导致图片滑动
        // 调整为在缩放动作时不处理滑动操作
        interactive: !IV.GStatus.fullScreenAnimating && IV.GStatus.viewInteractive
        model: IV.ImageInfo.Ready === targetImageInfo.status ? targetImageInfo.frameCount : 0
        delegate: Loader {
            width: multiImageDelegate.width
            height: multiImageDelegate.height
            active: {
                if (ListView.isCurrentItem) {
                    return true
                }
                if (multiImageView.currentIndex - 1 === index
                        || multiImageView.currentIndex + 1 === index) {
                    return true
                }
                return false
            }
            asynchronous: multiImageView.currentIndex - 1 === index
                          || multiImageView.currentIndex + 1 === index

            sourceComponent: Item {
                id: imageItem

                property bool isCurrentImage: parent.ListView.isCurrentItem

                height: parent.height
                width: parent.width

                onIsCurrentImageChanged: {
                    if (imageItem.isCurrentImage) {
                        multiImageDelegate.targetImage = image
                        multiImageDelegate.inputHandler = imageInput
                    }

                    multiImageDelegate.reset()
                }

                Image {
                    id: image

                    // TIF 图片暂无旋转功能
                    height: parent.height
                    width: parent.width
                    asynchronous: true
                    cache: false
                    fillMode: Image.PreserveAspectFit
                    mipmap: true
                    smooth: true
                    scale: 1.0
                    source: "image://ImageLoad/" + multiImageDelegate.source + "#frame_" + index

                    Binding {
                        target: multiImageDelegate
                        property: "status"
                        value: image.status
                        when: imageItem.isCurrentImage
                    }
                }

                // 和 image 保持同一层级
                ImageInputHandler {
                    id: imageInput

                    anchors.fill: parent
                    targetImage: image.status === Image.Ready ? image : null
                }

                Component.onCompleted: {
                    imageInput.reset()
                }
            }
        }

        onCurrentIndexChanged: {
            if (isCurrentImage && currentIndex != IV.GControl.currentFrameIndex) {
                IV.GControl.currentFrameIndex = currentIndex
            }
        }

        onMovementStarted: {
            IV.GStatus.viewFlicking = true
        }

        onMovementEnded: {
            IV.GStatus.viewFlicking = false
        }
    }
}
