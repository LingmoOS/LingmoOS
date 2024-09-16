// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Effects
import Qt5Compat.GraphicalEffects
import org.deepin.dtk 1.0 as DTK
import org.deepin.image.viewer 1.0 as IV

BaseThumbnailDelegate {
    id: normalThumbnailDelegate

    // 判断是否为多页图
    property bool isMultiImage: IV.Types.MultiImage === img.type

    states: [
        // 激活状态
        State {
            name: "active"
            when: isCurrentItem && !isMultiImage

            PropertyChanges {
                height: 50
                imgRadius: 4
                target: normalThumbnailDelegate
                width: height
                y: 15
            }

            PropertyChanges {
                border.width: imgRadius
                opacity: 1
                target: normalThumbnailDelegate.shader
                visible: true
                z: 1
            }
        }
    ]

    onIsCurrentItemChanged: {
        rotateAnimation.complete();
    }

    Item {
        id: imageItem

        anchors.fill: parent
        visible: true

        ThumbnailImage {
            id: img

            anchors.centerIn: parent
            height: parent.height
            layer.enabled: true
            source: normalThumbnailDelegate.source
            width: parent.width

            // 1. 使用 layer 而不是外部 OpacityMask 组件蒙版以在动画时仍支持圆角
            // 2. Qt6 下的 MultiEffect 圆角效果没有 OpacityMask 平滑
            layer.effect: OpacityMask {
                maskSource: Rectangle {
                    height: img.height
                    radius: imgRadius
                    width: img.width
                }
            }
        }
    }

    RotationAnimation {
        id: rotateAnimation

        alwaysRunToEnd: true
        duration: IV.GStatus.animationDefaultDuration
        easing.type: Easing.OutExpo
        target: img.image

        onRunningChanged: {
            if (!running) {
                img.reset();
            }
        }
    }

    // 执行旋转操作后，重新读取缓存数据，更新图片状态
    Connections {
        function onCurrentRotationChanged() {
            // Note: 确保缓存中的数据已刷新后更新界面
            // 0 为复位，缓存中的数据已转换，无需再次加载
            if (0 !== IV.GControl.currentRotation) {
                rotateAnimation.to = IV.GControl.currentRotation;
                rotateAnimation.start();
            }
        }

        enabled: isCurrentItem
        target: IV.GControl
    }

    // 图片数角标
    Loader {
        id: anchorLoader

        // 非多页图无需实例化
        active: isMultiImage
        height: 14
        // 仅多页图显示角标(为焦点时不加载)
        visible: isMultiImage
        width: Math.max(20, implicitWidth)

        sourceComponent: Rectangle {
            id: anchorRect

            anchors.fill: parent
            radius: 4

            // 多页图图片数角标
            DTK.Label {
                id: anchorLabel

                anchors.fill: parent
                bottomPadding: 3
                font.pixelSize: 11
                font.weight: Font.DemiBold
                horizontalAlignment: Text.AlignHCenter
                leftPadding: 2
                rightPadding: 2
                // 取得当前索引的图片帧号
                text: (img.frameCount <= 999) ? img.frameCount : "999+"
                topPadding: 3
                verticalAlignment: Text.AlignVCenter

                background: Rectangle {
                    implicitHeight: 14
                    implicitWidth: 14
                    radius: 4

                    gradient: Gradient {
                        GradientStop {
                            color: "#FFC3C3C3"
                            position: 0.0
                        }

                        GradientStop {
                            color: "#FFD8D8D8"
                            position: 1.0
                        }
                    }
                }
            }

            // 图片角标的内阴影
            InnerShadow {
                anchors.fill: anchorLabel
                color: Qt.rgba(0, 0, 0, 0.1)
                source: anchorLabel
                verticalOffset: -1
            }

            // 图片角标的外阴影
            DropShadow {
                anchors.fill: anchorLabel
                cached: true
                color: Qt.rgba(0, 0, 0, 0.3)
                radius: 2
                samples: 4
                source: anchorLabel
                verticalOffset: 1
            }
        }

        anchors {
            bottom: parent.bottom
            right: parent.right
        }
    }

    MouseArea {
        id: mouseArea

        anchors.fill: parent
        hoverEnabled: true

        onClicked: {
            // index 和 frameIndex 同时变更时必须一同设置
            IV.GControl.setIndexAndFrameIndex(index, 0);
        }
    }
}
