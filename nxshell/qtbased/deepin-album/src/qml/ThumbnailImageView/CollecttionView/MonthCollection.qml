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

import org.deepin.album 1.0 as Album

SwitchViewAnimation {
    id: monthView

    idName: "monthView"
    signal monthClicked(string year, string month)

    property real itemHeight: theView.width * 4 / 7
    property alias count: theModel.count
    function scrollToYear(year) {
        //搜索index
        var index = 0;
        for(var i = 0;i !== theModel.count; i++) {
            if(theModel.get(i).year === year) {
                index = i
                break
            }
        }

        //计算偏移
        var yValue = (itemHeight + theView.spacing) * index
        theView.contentY = 0
        theView.contentY = yValue
    }

    function flushModel() {
        if (!visible)
            return
        //0.清理
        theModel.clear()

        //1.获取月份
        var monthArray = albumControl.getMonths()
        var yearMonthArray = new Array
        for(var j = 0;j !== monthArray.length;++j) {
            var data = monthArray[j].split("-")
            yearMonthArray.push(data)
        }

        //2.获取item count并构建model
        for(var i = 0;i !== monthArray.length;++i) {
            var itemCount = albumControl.getMonthCount(yearMonthArray[i][0], yearMonthArray[i][1])
            theModel.append({year: yearMonthArray[i][0], month: yearMonthArray[i][1], itemCount: itemCount})
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
            id: delegateMain
            width: theView.width
            height: itemHeight + GStatus.collectionTopMargin

            property var paths: albumControl.getMonthPaths(year, month)

            MonthImage {
                id: image
                anchors.verticalCenter: parent.verticalCenter
                clip: true
                anchors {
                    leftMargin: -1
                }
                width: parent.width + 2
                height: itemHeight
                paths: delegateMain.paths
                displayFlushHelper: theView.displayFlushHelper

                visible: false
                Component.onCompleted: {
                    image.createImage()
                }
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
                //color: "transparent"
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

                border.color: Qt.rgba(0, 0, 0, 0.2)
                border.width: 1
                visible: true
                radius: 18
            }

            Label {
                id: monthLabel
                font: DTK.fontManager.t3
                text: qsTr("%1/%2").arg(year).arg(Number(month))
                color: "#FFFFFF"
                anchors {
                    top: image.top
                    topMargin: 25
                    left: image.left
                    leftMargin: 25
                }
            }

            Rectangle {
                id: itemCountLabel
                visible: itemCount > 6
                color: "#000000"
                radius: 20
                opacity: 0.7
                width: 60
                height: 30

                Text {
                    anchors.centerIn: parent
                    text: itemCount - 6 < 99 ? itemCount - 6 : "99+"
                    color: "#FFFFFF"
                }

                anchors {
                    right: parent.right
                    rightMargin: 10
                    bottom: parent.bottom
                    bottomMargin: 10
                }
            }

            MouseArea {
                anchors.fill: image
                onClicked: {
                    monthClicked(year, month)
                    forceActiveFocus()
                }
            }
        }
    }

    Component.onCompleted: {
        GStatus.sigFlushAllCollectionView.connect(flushModel)
    }
}
