// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Window
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Dialogs
import Qt5Compat.GraphicalEffects
import org.deepin.dtk 1.0
import "../../Control"
import "../../Control/ListView"
import "../../Control/Animation"
import "../../"

SwitchViewAnimation {
    id: yearView

    signal yearClicked(string year)
    property alias count: theModel.count
    property real itemHeight: theView.width * 4 / 7

    function flushModel() {
        if (!visible)
            return
        //0.清理
        theModel.clear()

        //1.获取年份
        var yearArray = albumControl.getYears()

        //2.获取item count并构建model
        for(var i = 0;i !== yearArray.length;++i) {
            var itemCount = albumControl.getYearCount(yearArray[i])
            theModel.append({year: yearArray[i], itemCount: itemCount})
        }
    }

    ListModel {
        id: theModel
    }

    ListView {
        property double displayFlushHelper: 0

        id: theView
        model: theModel
        clip: true
        delegate: theDelegate
        spacing: 0

        width: parent.width / 3 * 2
        height: parent.height + GStatus.statusBarHeight
        anchors {
            top: parent.top
            horizontalCenter: parent.horizontalCenter
        }
    }

    Component {
        id: theDelegate

        Item {
            width: theView.width
            height: itemHeight + GStatus.collectionTopMargin

            property string yearPath: albumControl.getYearCoverPath(year)
            Image {
                id: image

                anchors.verticalCenter: parent.verticalCenter
                //因为新版本的Qt的图片缓存机制，导致相同路径的图片只会加载一次,source要改变才能刷新图片，所以尾部添加itemCount。如果需要数量相同也能刷新，则在尾部添加随机数
                source: "image://collectionPublisher/" + theView.displayFlushHelper.toString() + "_Y_" + year + "_0" + "_" + itemCount
                width: parent.width
                height: itemHeight
                visible: false
                smooth: true
                antialiasing: true
                Rectangle {
                    anchors.fill: parent
                    color: Qt.rgba(0, 0, 0, 0)
                }
                fillMode: Image.PreserveAspectCrop
            }

            Rectangle {
                id: mask
                anchors.fill: image
                radius: 18
                visible: false
            }

            OpacityMask{
                id: opacityMask
                anchors.fill: image
                source: image
                maskSource: mask
                antialiasing: true
                smooth: true
            }

            //border and shadow
            Rectangle {
                id: borderRect
                anchors.fill: image
                gradient: Gradient {
                    GradientStop {
                        position: 0.0
                        color: Qt.rgba(0,0,0,0.4)
                    }
                    GradientStop {
                        position: 0.25
                        color: Qt.rgba(0,0,0,0)
                    }
                }
                border.color: Qt.rgba(0, 0, 0, 0.1)
                border.width: 1
                visible: true
                radius: 18
            }

            Label {
                id: yearLabel
                font: DTK.fontManager.t3
                text: qsTr("%1").arg(year)
                color: "#FFFFFF"
                anchors {
                    top: image.top
                    topMargin: 25
                    left: image.left
                    leftMargin: 25
                }
            }

            Label {
                id: itemCountLabel
                font: DTK.fontManager.t6
                text: itemCount > 1 ? qsTr("%1 items").arg(itemCount) : qsTr("1 item") //itemCount为0的时候不会显示出来
                color: yearLabel.color
                anchors {
                    top: yearLabel.bottom
                    topMargin: 5
                    left: yearLabel.left
                }
            }

            MouseArea {
                anchors.fill: image
                onClicked: { //double click 切换动画不生效
                    yearClicked(year)
                    forceActiveFocus()
                }
            }
        }
    }

    Component.onCompleted: {
        GStatus.sigFlushAllCollectionView.connect(flushModel)
    }
}
