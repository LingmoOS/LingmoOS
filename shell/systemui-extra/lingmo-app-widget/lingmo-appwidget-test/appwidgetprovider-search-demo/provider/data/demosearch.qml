/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: wangyan <wangyan@kylinos.cn>
 *
 */

import QtQuick 2.6
import QtQuick.Window 2.2
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.0
import org.lingmo.appwidget 1.0

AppWidget {
    id: window
    //注1:appname需要小插件赋值
    appname: "demosearch"
    //小插件的位置应该设置为居中
    anchors.centerIn: parent
    //注2:小插件应考虑在不同缩放以及分辨率下的适配
    //可以采用如下方式计算缩放比scalefactor（仅供参考，只要能实现小插件根据父类窗口缩放即可）
    width: 640 * scalefactor
    height: 100 * scalefactor
    property double scalefactor: 1.00
    visible: true
    //注3:qmlenable为当前小插件是否可以进行操作的标志，当clickable为 false时，
    //小插件不可点击，不能接收焦点等。该状态的决定由使用端传递并更新
    property bool clickable: true
    Rectangle {
        id: searchBar
        anchors.centerIn: parent
        width: 400 * scalefactor
        height: 35 * scalefactor
        border.color: "#fc5531"
        border.width: 1
        radius: searchBar.height / 2
        Rectangle {
            id: btn
            width: 80 * scalefactor
            height: searchBar.height
            anchors.right: parent.right
            anchors.top: parent.top
            color: "#fc5531"
            layer.enabled: true
            layer.effect: OpacityMask {
                maskSource: Rectangle {
                    width: 80 * scalefactor
                    height: btn.height
                    radius: btn.height / 2
                    //左侧
                    Rectangle {
                        width: 20 * scalefactor
                        height: btn.height
                    }
                }
            }
            Image {
                id: image_search
                width: 20 * scalefactor
                height: 20 * scalefactor
                x: 10
                anchors.verticalCenter: parent.verticalCenter
                source: "file:///usr/share/appwidgetdemo/search/lingmo-search.svg"
            }
            Text {
                id: searchTxt
                color: "White"
                text: qsTr("search")
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: image_search.right
                anchors.leftMargin: 1
                font.pointSize: 12 * scalefactor
                font.bold: true
            }
        }
    }
    MouseArea {
        anchors.fill: parent
        //注4：如果使用MouseArea需要注意enabled需要与window.clickable一致
        enabled: window.clickable
        //注5：事件透传，在小插件中部分事件需要透传到使用端处理，如onPressAndHold，其他的暂时未发现需要设置
        //如需要实现onClicked，则注意onPressed一定不要透传，否则小插件自身将不能得到点击事件
        propagateComposedEvents: true
        onClicked: {
            //点击需要进行的跳转
            qmlclickedsignal("demosearch", "clicked");
            mouse.accepted = false
        }
        //注6：onPressAndHold事件需要透传，否则将不能实现长按进入编辑状态（不能长按进行拖动）
        onPressAndHold: {
            mouse.accepted = false
        }
    }

    Component.onCompleted: {
        //注7:初始化缩放比
        initScalefactor()
        //监听使用端状态改变信号，用于更新clickable
        userStatusChangedSignal.connect(onUserStatusChanged)
    }
    function initScalefactor() {
        //按宽计算缩放比例,计算在这个比例系数下的小插件宽高，如果高超出父类的高，则更换比例系数的计算方式，按高计算缩放比例
        //console.log("父类窗口大小", window.parent.width,window.parent.height);
        if(window.parent.width == 0 || window.parent.height == 0) {
            window.scalefactor = 1.0
            console.log("父类窗口大小为 0 默认缩放比为1.0")
            return
        }
        //640为默认情况下demosearch的宽度
        window.scalefactor = window.parent.width / 640
        //console.log("当前比例系数", window.scalefactor);
        //100为默认情况下demosearch的高度
        var tw = window.scalefactor * 640
        var th = window.scalefactor * 100
        //console.log("大小：", tw,th ,window.height);
        //当按宽缩放时，父类窗口的高小于小插件缩放后实际的高时则重新以高度计算缩放比
        if(th > window.parent.height) {
            window.scalefactor = window.parent.height / 100
            //console.log("以宽度计算比例系数", appwidget.scalefactor);
        }
    }
    function onUserStatusChanged(status) {
        console.log("qml Appwidget demosearch user status changed ", status);
        if(status === "Normal") {
            window.clickable = true;
        }else if(status === "Editable") {
            window.clickable = false;
        }
    }
}


