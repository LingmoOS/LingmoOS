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
import org.lingmo.appwidget 1.0

AppWidget {
    id: window
    //注1：appname必须赋值
    appname: "democlock"
    //注2：小插件的位置应该设置为居中
    anchors.centerIn: parent
    //注3:小插件应考虑在不同缩放以及分辨率下的适配
    //可以采用如下方式计算缩放比scalefactor（仅供参考，只要能实现小插件根据父类窗口缩放即可）
    width: 740 * scalefactor
    height: 680 * scalefactor
    property double scalefactor: 1.00
    visible: true
    //注4:clickable为当前小插件是否可以进行操作的标志，当clickable为 false时，
    //小插件不可点击，不能接收焦点等。该状态的决定由使用端传递并更新
    property bool clickable: true
    property var date : ""
    property var aclock : ""
    Canvas {
        id: canvas
        anchors.fill: parent
        width: 740 * scalefactor
        height: 680 * scalefactor
        onPaint: {
            var ctx = getContext("2d");
            drawBack(ctx);
            drawDot(ctx);

            var date = new Date();
            var hours =  date.getHours()
            hours = hours % 12;
            var minutes =  date.getMinutes()
            var seconds = date.getUTCSeconds();
            var timestr = date.toLocaleString("yyyy-MM-dd hh:mm:ss");
            drawSecond(seconds, ctx);
            drawHour(hours, minutes, ctx);
            drawMinute(minutes, seconds, ctx);
            drawSecond(seconds, ctx);
        }

        Text {
            id: datetext
            color: "black"
            text: qsTr("日期：" + date)
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.leftMargin: 1
            y : 350 * scalefactor
            font.pointSize: 12 * scalefactor
            font.bold: true
        }
        Text {
            id: clocktext
            color: "black"
            text: qsTr("日期：" + aclock)
            wrapMode: Text.WordWrap
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.leftMargin: 1
            y : 370 * scalefactor
            font.pointSize: 12 * scalefactor
            font.bold: true
        }
    }
    MouseArea {
        id: mousearea
        anchors.fill: parent
        //注5：如果使用MouseArea需要注意enabled需要与window.clickable一致
        enabled: window.clickable
        //注6：事件透传，在小插件中部分事件需要透传到使用端处理，如onPressAndHold，其他的暂时未发现需要设置
        //如需要实现onClicked，则注意onPressed一定不要透传，否则小插件自身将不能得到点击事件
        propagateComposedEvents: true
        onClicked: {
            //点击需要进行的跳转
            window.qmlclickedsignal("democlock", "clicked");
            mouse.accepted = false
        }
        //注7：onPressAndHold事件需要透传，否则将不能实现长按进入编辑状态（不能长按进行拖动）
        onPressAndHold: {
            mouse.accepted = false
        }
    }
    function drawBack(ctx) {
        var r = 200 * scalefactor;
        ctx.save();
        ctx.beginPath();
        ctx.translate(width / 2,height / 2);
        ctx.clearRect(-r, -r, r * 2, r * 2);
        ctx.lineWidth = 10;
        ctx.arc(0, 0, r - 5, 0, Math.PI * 2, false);  //外圆
        ctx.stroke();

        //画文本
        var numbers = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12];
        ctx.font = "18px Arial";
        ctx.textAlign = "center";
        ctx.textBaseline = "middle";
        for(var i = 0; i < 12; ++i) {
            var rad = 2 * Math.PI / 12 * numbers[i] - 3.14 / 2;
            var x = Math.cos(rad) * (r-30);
            var y = Math.sin(rad) * (r-30);
            ctx.fillText(numbers[i], x, y);
        }

        //画刻度
        ctx.lineWidth = 2;

        for(var i = 0; i < 60; ++i) {
            ctx.beginPath();
            var rad = 2 * Math.PI / 60 * i;
            var x = Math.cos(rad) * (r - 15);
            var y = Math.sin(rad) * (r - 15);
            var x2 = Math.cos(rad) * (r - 10);
            var y2 = Math.sin(rad) * (r - 10);
            //通过画线也可以
            if(i%5 === 0) {
                ctx.strokeStyle = "#000000";
            }else {
                ctx.strokeStyle = "#989898";
            }
            ctx.moveTo(x, y);
            ctx.lineTo(x2, y2);
            ctx.stroke();
        }
        ctx.restore();
    }

    function drawHour(hour, minute, ctx) {
        var r = 200 * scalefactor;
        ctx.save();
        ctx.beginPath(); //必须写
        ctx.lineWidth = 7;
        ctx.lineCap = "round";
        ctx.translate(width / 2,height / 2);

        var rad = Math.PI * 2 / 12 * hour + Math.PI * 2 * minute / 12 / 60 - Math.PI;
        ctx.rotate(rad);
        ctx.moveTo(0, -10);
        ctx.lineTo(0, r / 2 - 10);
        ctx.stroke();   //必须写（填充方式）
        ctx.restore();
    }
    function drawMinute(minute, second, ctx) {
        var r = 200 * scalefactor;
        ctx.save();
        ctx.beginPath(); //必须写
        ctx.lineWidth = 5;
        ctx.lineCap = "round";
        ctx.translate(width / 2,height / 2);

        var rad = Math.PI * 2 * minute / 60 + Math.PI * 2 * second / 60 / 60 - Math.PI;
        ctx.rotate(rad);
        ctx.moveTo(0, -10);
        ctx.lineTo(-10, r * 3 / 5 - 10);
        ctx.stroke();   //必须写（填充方式）
        ctx.restore();
    }
    function drawSecond(second, ctx) {
        var r = 200 * scalefactor;
        ctx.save();
        ctx.beginPath(); //必须写
        ctx.lineWidth = 3;
        ctx.lineCap = "round";
        ctx.translate(width / 2, height / 2);
        ctx.strokeStyle="#ff0000";
        var rad = Math.PI * 2 * second / 60 - Math.PI;
        ctx.rotate(rad);
        ctx.moveTo(0, -20);
        ctx.lineTo(0, r * 4 / 5 - 10);
        ctx.stroke();   //必须写（填充方式）
        ctx.restore();
    }
    function drawDot(ctx) {
        ctx.save();
        ctx.beginPath(); //必须写
        ctx.translate(width / 2, height / 2);
        ctx.fillStyle = "#ffffff";
        ctx.arc(0, 0, 3, 0, Math.PI * 2);
        ctx.fill();     //必须写（填充方式）
        ctx.restore();
    }
    Component.onCompleted: {
        console.log("qml 接收到数据更新信号");
        window.update.connect(window.onupdate);
        //注8:初始化缩放比
        initScalefactor()
        //注9:监听使用端状态改变信号，用于更新clickable
        userStatusChangedSignal.connect(onUserStatusChanged)

    }
    function onupdate() {
        for (var i in window.datavalue)
        {
            var info = window.datavalue[i];
            if( i === "date") {
                console.log("date", info);
                datetext.text = qsTr("日期：" + info)
            }
            if( i === "AlarmClock") {
                console.log("alarmClock", info);
                clocktext.text = qsTr("闹钟" + info)
            }
        }
    }
    //定时器
    Timer {
        interval: 1000; running: true; repeat: true
        onTriggered: canvas.requestPaint();
    }

    function initScalefactor() {
        //注10：按宽计算缩放比例,计算在这个比例系数下的小插件宽高，如果高超出父类的高，则更换比例系数的计算方式，按高计算缩放比例
        //console.log("父类窗口大小", window.parent.width,window.parent.height);
        if(window.parent.width == 0 || window.parent.height == 0) {
            window.scalefactor = 1.0
            console.log("父类窗口大小为 0 默认缩放比为1.0")
            return
        }
        //注11：640为默认情况下demosearch的宽度
        window.scalefactor = window.parent.width / 740
        //console.log("当前比例系数", window.scalefactor);
        //注12：100为默认情况下demosearch的高度
        var tw = window.scalefactor * 740
        var th = window.scalefactor * 680
        //console.log("大小：", tw,th ,window.height);
        //注13：当按宽缩放时，父类窗口的高小于小插件缩放后实际的高时则重新以高度计算缩放比
        if(th > window.parent.height) {
            window.scalefactor = window.parent.height / 680
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
