/*
 * Copyright (C) 2021 LingmoOS Team.
 *
 * Author:     revenmartin <revenmartin@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import LingmoUI 1.0 as LingmoUI
import QtGraphicalEffects 1.0
import Lingmo.Settings 1.0
import "../"

ItemPage {
    headerTitle: qsTr("Date & Time")

    TimeZoneDialog {
        id: timeZoneDialog
    }

    TimeZoneMap {
        id: timeZoneMap
    }

    Time {
        id: time
    }

    Scrollable {
        anchors.fill: parent
        contentHeight: layout.implicitHeight

        ColumnLayout {
            id: layout
            anchors.fill: parent
            spacing: LingmoUI.Units.largeSpacing * 2

            RowLayout {
                anchors.fill: parent
                spacing: 20

                Rectangle {
                    Layout.preferredWidth: Math.min(parent.height * 0.7, parent.width * 0.4)
                    Layout.preferredHeight: Layout.preferredWidth
                    Layout.alignment: Qt.AlignVCenter
                    color: "transparent"  // 移除背景色

                    // 添加玻璃效果背景
                    Rectangle {
                        id: glassEffect
                        anchors.fill: parent
                        radius: width / 2
                        color: LingmoUI.Theme.backgroundColor
                        opacity: LingmoUI.Theme.darkMode ? 0.2 : 0.7

                        // 玻璃模糊效果
                        layer.enabled: true
                        layer.effect: FastBlur {
                            radius: 32
                        }

                        // 渐变效果
                        Rectangle {
                            anchors.fill: parent
                            radius: parent.radius
                            gradient: Gradient {
                                GradientStop { 
                                    position: 0.0
                                    color: LingmoUI.Theme.darkMode ? 
                                        Qt.rgba(1, 1, 1, 0.1) : Qt.rgba(1, 1, 1, 0.3)
                                }
                                GradientStop { 
                                    position: 1.0
                                    color: LingmoUI.Theme.darkMode ? 
                                        Qt.rgba(1, 1, 1, 0.05) : Qt.rgba(1, 1, 1, 0.1)
                                }
                            }
                        }

                        // 边框效果
                        Rectangle {
                            anchors.fill: parent
                            radius: parent.radius
                            color: "transparent"
                            border.width: 1
                            border.color: LingmoUI.Theme.darkMode ? 
                                Qt.rgba(1, 1, 1, 0.1) : Qt.rgba(1, 1, 1, 0.3)
                        }
                    }

                    // 时钟内容
                    Item {
                        id: clock
                        anchors.centerIn: parent
                        width: Math.min(parent.width, parent.height) - 40
                        height: width

                        // 修改刻度绘制
                        Canvas {
                            anchors.fill: parent
                            z: 1
                            onPaint: {
                                var ctx = getContext("2d");
                                var centerX = width / 2;
                                var centerY = height / 2;
                                var radius = Math.min(width, height) / 2;

                                ctx.reset();
                                ctx.save();
                                ctx.translate(centerX, centerY);

                                // 绘制刻度
                                for (var i = 0; i < 60; i++) {
                                    var angle = i * 6 * Math.PI / 180;
                                    var isHour = i % 5 === 0;
                                    
                                    ctx.beginPath();
                                    ctx.lineWidth = isHour ? 2 : 1;
                                    ctx.strokeStyle = LingmoUI.Theme.textColor;
                                    ctx.globalAlpha = isHour ? 
                                        (LingmoUI.Theme.darkMode ? 0.6 : 0.8) : 
                                        (LingmoUI.Theme.darkMode ? 0.2 : 0.3);
                                    
                                    var innerRadius = radius - (isHour ? radius * 0.15 : radius * 0.1);
                                    var outerRadius = radius - radius * 0.05;
                                    
                                    ctx.moveTo(
                                        innerRadius * Math.sin(angle),
                                        -innerRadius * Math.cos(angle)
                                    );
                                    ctx.lineTo(
                                        outerRadius * Math.sin(angle),
                                        -outerRadius * Math.cos(angle)
                                    );
                                    
                                    ctx.stroke();
                                }

                                // 添加圆形装饰
                                ctx.beginPath();
                                ctx.arc(0, 0, radius - radius * 0.02, 0, Math.PI * 2);
                                ctx.strokeStyle = LingmoUI.Theme.textColor;
                                ctx.globalAlpha = LingmoUI.Theme.darkMode ? 0.05 : 0.1;
                                ctx.lineWidth = 1;
                                ctx.stroke();

                                ctx.restore();
                            }

                            // 主题改变时重绘
                            Connections {
                                target: LingmoUI.Theme
                                function onDarkModeChanged() {
                                    parent.requestPaint()
                                }
                            }
                        }

                        // 数字
                        Repeater {
                            model: 12
                            Text {
                                x: clock.width / 2 - width / 2 + Math.sin(index * 30 * Math.PI / 180) * (clock.width * 0.35)
                                y: clock.height / 2 - height / 2 - Math.cos(index * 30 * Math.PI / 180) * (clock.height * 0.35)
                                text: index == 0 ? "12" : index
                                font {
                                    pixelSize: clock.width * 0.08
                                    family: "Arial"
                                }
                                color: LingmoUI.Theme.textColor
                                opacity: 0.8
                                z: 2
                            }
                        }

                        // 修改指针样式
                        Rectangle {
                            id: hourHand
                            width: clock.width * 0.04
                            height: clock.height * 0.25
                            color: LingmoUI.Theme.textColor
                            opacity: LingmoUI.Theme.darkMode ? 0.7 : 0.9
                            radius: width / 2
                            x: clock.width / 2 - width / 2
                            y: clock.height / 2 - height
                            transformOrigin: Item.Bottom
                            antialiasing: true

                            // 添加阴影效果
                            layer.enabled: true
                            layer.effect: DropShadow {
                                radius: 4
                                samples: 8
                                color: LingmoUI.Theme.darkMode ? 
                                    Qt.rgba(0, 0, 0, 0.5) : Qt.rgba(0, 0, 0, 0.3)
                            }
                        }

                        Rectangle {
                            id: minuteHand
                            width: clock.width * 0.03
                            height: clock.height * 0.35
                            color: LingmoUI.Theme.textColor
                            opacity: LingmoUI.Theme.darkMode ? 0.6 : 0.7
                            radius: width / 2
                            x: clock.width / 2 - width / 2
                            y: clock.height / 2 - height
                            transformOrigin: Item.Bottom
                            antialiasing: true

                            layer.enabled: true
                            layer.effect: DropShadow {
                                radius: 4
                                samples: 8
                                color: LingmoUI.Theme.darkMode ? 
                                    Qt.rgba(0, 0, 0, 0.5) : Qt.rgba(0, 0, 0, 0.3)
                            }
                        }

                        Rectangle {
                            id: secondHand
                            width: clock.width * 0.02
                            height: clock.height * 0.4
                            color: LingmoUI.Theme.highlightColor
                            opacity: LingmoUI.Theme.darkMode ? 0.8 : 1.0
                            radius: width / 2
                            x: clock.width / 2 - width / 2
                            y: clock.height / 2 - height
                            transformOrigin: Item.Bottom
                            antialiasing: true

                            layer.enabled: true
                            layer.effect: DropShadow {
                                radius: 4
                                samples: 8
                                color: LingmoUI.Theme.darkMode ? 
                                    Qt.rgba(0, 0, 0, 0.5) : Qt.rgba(0, 0, 0, 0.3)
                            }
                        }

                        // 修改中心点
                        Rectangle {
                            width: clock.width * 0.08
                            height: width
                            radius: width / 2
                            color: LingmoUI.Theme.highlightColor
                            anchors.centerIn: parent
                            z: 4

                            // 内圈装饰
                            Rectangle {
                                width: parent.width * 0.6
                                height: width
                                radius: width / 2
                                color: LingmoUI.Theme.darkMode ? 
                                    Qt.rgba(1, 1, 1, 0.2) : "white"
                                anchors.centerIn: parent
                            }

                            layer.enabled: true
                            layer.effect: DropShadow {
                                radius: 8
                                samples: 16
                                color: LingmoUI.Theme.darkMode ? 
                                    Qt.rgba(0, 0, 0, 0.6) : Qt.rgba(0, 0, 0, 0.3)
                            }
                        }
                    }
                }

                // 右侧时间日期显示
                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.preferredWidth: parent.width * 0.5  // 确保右侧有足够的宽度
                    spacing: 20

                    // 顶部弹性空间
                    Item {
                        Layout.fillHeight: true
                        Layout.preferredHeight: 1  // 权重为1
                    }

                    // 时间显示
                    Label {
                        id: timeLabel
                        Layout.alignment: Qt.AlignHCenter
                        font {
                            pixelSize: Math.min(clock.width * 0.3, 72)
                            family: "Arial"
                        }
                        color: LingmoUI.Theme.textColor
                        text: {
                            var date = new Date();
                            return date.toLocaleTimeString(Qt.locale(), "HH:mm:ss");
                        }
                    }

                    // 日期显示
                    Label {
                        id: dateLabel
                        Layout.alignment: Qt.AlignHCenter
                        font {
                            pixelSize: Math.min(clock.width * 0.1, 24)
                            family: "Arial"
                        }
                        color: LingmoUI.Theme.textColor
                        opacity: 0.8
                        text: {
                            var date = new Date();
                            return date.toLocaleDateString(Qt.locale(), Locale.LongFormat);
                        }
                        horizontalAlignment: Text.AlignHCenter
                        Layout.preferredWidth: parent.width * 0.8
                        wrapMode: Text.Wrap
                    }

                    // 底部弹性空间
                    Item {
                        Layout.fillHeight: true
                        Layout.preferredHeight: 1  // 权重为1
                    }
                }
            }
        

            Timer {
                interval: 16  // 约60fps的刷新率
                running: true
                repeat: true
                onTriggered: {
                    var date = new Date()
                    hourHand.rotation = (date.getHours() % 12 + date.getMinutes() / 60) * 30
                    minuteHand.rotation = date.getMinutes() * 6
                    secondHand.rotation = (date.getSeconds() + date.getMilliseconds() / 1000) * 6
                    timeLabel.text = date.toLocaleTimeString(Qt.locale(), "HH:mm:ss")
                    dateLabel.text = date.toLocaleDateString(Qt.locale(), Locale.LongFormat)
                }
            }

            RoundedItem {
                spacing: LingmoUI.Units.largeSpacing * 1.5

                RowLayout {
                    Label {
                        text: qsTr("Auto Sync")
                    }

                    Item {
                        Layout.fillWidth: true
                    }

                    Switch {
                        Layout.fillHeight: true
                        rightPadding: 0
                        rightInset: 0
                        checked: time.useNtp
                        onCheckedChanged: time.useNtp = checked
                    }
                }

                RowLayout {
                    Label {
                        text: qsTr("24-Hour Time")
                    }

                    Item {
                        Layout.fillWidth: true
                    }

                    Switch {
                        Layout.fillHeight: true
                        rightPadding: 0
                        rightInset: 0
                        checked: time.twentyFour
                        onCheckedChanged: time.twentyFour = checked
                    }
                }
            }

            StandardButton {
                Layout.fillWidth: true
                text: ""
                // onClicked: timeZoneDialog.visibility = "Maximized"
                onClicked: timeZoneDialog.show()

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: LingmoUI.Units.largeSpacing * 1.5
                    anchors.rightMargin: LingmoUI.Units.largeSpacing * 1.5

                    Label {
                        text: qsTr("Time Zone")
                    }

                    Item {
                        Layout.fillWidth: true
                    }

                    Label {
                        text: timeZoneMap.currentTimeZone
                    }
                }
            }
        }
    }
}
