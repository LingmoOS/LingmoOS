/*
 * Copyright (C) 2025 Lingmo OS Team.
 *
 * Author:     Reion Wong <aj@lingmo.org>
 * Author:     Lingmo OS Team <team@lingmo.org>
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
import QtQuick.Window 2.12
import QtQuick.Layouts 1.12
import QtGraphicalEffects 1.0

import Lingmo.StatusBar 1.0
import Lingmo.Audio 1.0
import LingmoUI 1.0 as LingmoUI

DateTimeDialog {
    id: control

    width: 450
    height: control.width/2

    property var margin: 4 * Screen.devicePixelRatio
    property point position: Qt.point(0, 0)
    property var defaultSink: paSinkModel.defaultSink

    property bool bluetoothDisConnected: Bluez.Manager.bluetoothBlocked
    property var defaultSinkValue: defaultSink ? defaultSink.volume / PulseAudio.NormalVolume * 100.0 : -1

    property var borderColor: windowHelper.compositing ? LingmoUI.Theme.darkMode ? Qt.rgba(255, 255, 255, 0.3)
                                                                  : Qt.rgba(0, 0, 0, 0.2) : LingmoUI.Theme.darkMode ? Qt.rgba(255, 255, 255, 0.15)
                                                                                                                  : Qt.rgba(0, 0, 0, 0.15)

    onWidthChanged: adjustCorrectLocation()
    onHeightChanged: adjustCorrectLocation()
    onPositionChanged: adjustCorrectLocation()

    color: "transparent"

    LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft
    LayoutMirroring.childrenInherit: true

    function adjustCorrectLocation() {
        var posX = control.position.x
        var posY = control.position.y

        if (posX + control.width >= StatusBar.screenRect.x + StatusBar.screenRect.width)
            posX = StatusBar.screenRect.x + StatusBar.screenRect.width - control.width - control.margin

        posY = rootItem.y + rootItem.height + control.margin

        control.x = posX
        control.y = posY
    }

    Brightness {
        id: brightness
    }

    LingmoUI.WindowBlur {
        view: control
        geometry: Qt.rect(control.x, control.y, control.width, control.height)
        windowRadius: _background.radius
        enabled: true
    }

    LingmoUI.WindowShadow {
        view: control
        geometry: Qt.rect(control.x, control.y, control.width, control.height)
        radius: _background.radius
    }

    Rectangle {
        id: _background
        anchors.fill: parent
        radius: windowHelper.compositing ? LingmoUI.Theme.bigRadius * 1.5 : 0
        color: LingmoUI.Theme.darkMode ? "#4D4D4D" : "#F0F0F0"
        opacity: windowHelper.compositing ? LingmoUI.Theme.darkMode ? 0.6 : 0.8 : 1.0
        antialiasing: true
        border.width: 1 / Screen.devicePixelRatio
        border.pixelAligned: Screen.devicePixelRatio > 1 ? false : true
        border.color: control.borderColor

        Behavior on color {
            ColorAnimation {
                duration: 200
                easing.type: Easing.Linear
            }
        }
    }

    ColumnLayout {
        id: _mainLayout
        anchors.fill: parent
        // anchors.margins: LingmoUI.Units.largeSpacing
        spacing: LingmoUI.Units.largeSpacing

        Item {
            id: topItem
            Layout.fillWidth: true
            height: control.width/2

            RowLayout {
                anchors.fill: parent
                spacing: 10

                Rectangle {
                    anchors.fill: parent
                    // color: "white"
                    color: "transparent"
                }

                Rectangle {
                    Layout.preferredWidth: Math.min(parent.height * 0.7, parent.width * 0.4)
                    Layout.preferredHeight: Layout.preferredWidth
                    Layout.alignment: Qt.AlignVCenter
                    color: "transparent"  // 移除背景色

                    Rectangle {
                        anchors.fill: parent
                        color: "white"
                        // color: "transparent"
                        radius: LingmoUI.Theme.bigRadius *20
                        opacity: LingmoUI.Theme.darkMode ? 0.2 : 0.7
                    }

                    // // 添加玻璃效果背景
                    // Rectangle {
                    //     id: glassEffect
                    //     anchors.fill: parent
                    //     radius: width / 2
                    //     color: LingmoUI.Theme.backgroundColor
                    //     opacity: LingmoUI.Theme.darkMode ? 0.2 : 0.7

                    //     // 玻璃模糊效果
                    //     layer.enabled: true
                    //     layer.effect: FastBlur {
                    //         radius: 32
                    //     }

                    //     // 渐变效果
                    //     Rectangle {
                    //         anchors.fill: parent
                    //         radius: parent.radius
                    //         gradient: Gradient {
                    //             GradientStop { 
                    //                 position: 0.0
                    //                 color: LingmoUI.Theme.darkMode ? 
                    //                     Qt.rgba(1, 1, 1, 0.1) : Qt.rgba(1, 1, 1, 0.3)
                    //             }
                    //             GradientStop { 
                    //                 position: 1.0
                    //                 color: LingmoUI.Theme.darkMode ? 
                    //                     Qt.rgba(1, 1, 1, 0.05) : Qt.rgba(1, 1, 1, 0.1)
                    //             }
                    //         }
                    //     }

                    //     // 边框效果
                    //     // Rectangle {
                    //     //     anchors.fill: parent
                    //     //     radius: parent.radius
                    //     //     color: "transparent"
                    //     //     border.width: 1
                    //     //     border.color: LingmoUI.Theme.darkMode ? 
                    //     //         Qt.rgba(1, 1, 1, 0.1) : Qt.rgba(1, 1, 1, 0.3)
                    //     // }
                    //     
                    // }

                    // 时钟内容
                    Item {
                        id: clock
                        anchors.centerIn: parent
                        width: Math.min(parent.width, parent.height) - 10
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
        }
    }

//     ColumnLayout {
//         id: _mainLayout
//         anchors.fill: parent
//         anchors.margins: LingmoUI.Units.largeSpacing
//         spacing: LingmoUI.Units.largeSpacing

//         Item {
//             id: topItem
//             Layout.fillWidth: true
//             height: 32

//             RowLayout {
//                 id: topItemLayout
//                 anchors.fill: parent
//                 anchors.rightMargin: LingmoUI.Units.largeSpacing
//                 spacing: LingmoUI.Units.largeSpacing

//                 Label {
//                     leftPadding: LingmoUI.Units.largeSpacing
//                     text: qsTr("Control Center")
//                     font.bold: true
//                     font.pointSize: 14
//                     Layout.fillWidth: true
//                 }

//                 IconButton {
//                     id: settingsButton
//                     implicitWidth: topItem.height
//                     implicitHeight: topItem.height
//                     Layout.alignment: Qt.AlignTop
//                     source: "qrc:/images/" + (LingmoUI.Theme.darkMode ? "dark/" : "light/") + "settings.svg"
//                     onLeftButtonClicked: {
//                         control.visible = false
//                         process.startDetached("lingmo-settings")
//                     }
//                 }

// //                IconButton {
// //                    id: shutdownButton
// //                    implicitWidth: topItem.height
// //                    implicitHeight: topItem.height
// //                    Layout.alignment: Qt.AlignTop
// //                    source: "qrc:/images/" + (LingmoUI.Theme.darkMode ? "dark/" : "light/") + "system-shutdown-symbolic.svg"
// //                    onLeftButtonClicked: {
// //                        control.visible = false
// //                        process.startDetached("lingmo-shutdown")
// //                    }
// //                }
//             }
//         }

//         Item {
//             id: cardItems
//             Layout.fillWidth: true
//             Layout.preferredHeight: Math.ceil(cardLayout.count / 4) * 110

//             property var cellWidth: cardItems.width / 4

//             Rectangle {
//                 anchors.fill: parent
//                 color: "white"
//                 radius: LingmoUI.Theme.bigRadius
//                 opacity: LingmoUI.Theme.darkMode ? 0.2 : 0.7
//             }

//             GridLayout {
//                 id: cardLayout
//                 anchors.fill: parent
//                 columnSpacing: 0
//                 columns: 4

//                 property int count: {
//                     var count = 0

//                     for (var i in cardLayout.children) {
//                         if (cardLayout.children[i].visible)
//                             ++count
//                     }

//                     return count
//                 }

//                 CardItem {
//                     id: wirelessItem
//                     Layout.fillHeight: true
//                     Layout.preferredWidth: cardItems.cellWidth
//                     icon: LingmoUI.Theme.darkMode || checked ? "qrc:/images/dark/network-wireless-connected-100.svg"
//                                                            : "qrc:/images/light/network-wireless-connected-100.svg"
//                     visible: enabledConnections.wirelessHwEnabled
//                     checked: enabledConnections.wirelessEnabled
//                     label: activeConnection.wirelessName ? activeConnection.wirelessName : qsTr("Wi-Fi")
//                     onClicked: nmHandler.enableWireless(!checked)
//                     onPressAndHold: {
//                         control.visible = false
//                         process.startDetached("lingmo-settings", ["-m", "wlan"])
//                     }
//                 }

//                 CardItem {
//                     id: bluetoothItem
//                     Layout.fillHeight: true
//                     Layout.preferredWidth: cardItems.cellWidth
//                     icon: LingmoUI.Theme.darkMode || checked ? "qrc:/images/dark/bluetooth-symbolic.svg"
//                                                            : "qrc:/images/light/bluetooth-symbolic.svg"
//                     checked: !control.bluetoothDisConnected
//                     label: qsTr("Bluetooth")
//                     visible: Bluez.Manager.adapters.length
//                     onClicked: control.toggleBluetooth()
//                     onPressAndHold: {
//                         control.visible = false
//                         process.startDetached("lingmo-settings", ["-m", "bluetooth"])
//                     }
//                 }

//                 CardItem {
//                     id: darkModeItem
//                     Layout.fillHeight: true
//                     Layout.preferredWidth: cardItems.cellWidth
//                     icon: LingmoUI.Theme.darkMode || checked ? "qrc:/images/dark/dark-mode.svg"
//                                                            : "qrc:/images/light/dark-mode.svg"
//                     checked: LingmoUI.Theme.darkMode
//                     label: qsTr("Dark Mode")
//                     onClicked: {
//                         appearance.switchDarkMode(!LingmoUI.Theme.darkMode)
//                         if (LingmoUI.Theme.darkMode) {
//                             appearance.lightThemeChanged()
//                         } else {
//                             appearance.darkThemeChanged()
//                         }
//                     }
//                 }

//                 CardItem {
//                     Layout.fillHeight: true
//                     Layout.preferredWidth: cardItems.cellWidth
//                     icon: LingmoUI.Theme.darkMode || checked ? "qrc:/images/dark/do-not-disturb.svg"
//                                                            : "qrc:/images/light/do-not-disturb.svg"
//                     checked: notifications.doNotDisturb
//                     label: qsTr("Do Not Disturb")
//                     onClicked: notifications.doNotDisturb = !notifications.doNotDisturb
//                 }

//                 CardItem {
//                     Layout.fillHeight: true
//                     Layout.preferredWidth: cardItems.cellWidth
//                     icon: LingmoUI.Theme.darkMode || checked ? "qrc:/images/dark/screenshot.svg"
//                                                            : "qrc:/images/light/screenshot.svg"
//                     checked: false
//                     label: qsTr("Screenshot")
//                     onClicked: {
//                         control.visible = false
//                         process.startDetached("lingmo-screenshot", ["-d", "500"])
//                     }
//                 }
//             }
//         }

//         MprisItem {
//             height: 96
//             Layout.fillWidth: true
//         }

//         Item {
//             id: brightnessItem
//             Layout.fillWidth: true
//             height: 40
//             visible: brightness.enabled

//             Rectangle {
//                 id: brightnessItemBg
//                 anchors.fill: parent
//                 color: "white"
//                 radius: LingmoUI.Theme.bigRadius
//                 opacity: LingmoUI.Theme.darkMode ? 0.2 : 0.7
//             }

//             RowLayout {
//                 anchors.fill: brightnessItemBg
//                 anchors.leftMargin: LingmoUI.Units.largeSpacing
//                 anchors.rightMargin: LingmoUI.Units.largeSpacing
//                 anchors.topMargin: LingmoUI.Units.smallSpacing
//                 anchors.bottomMargin: LingmoUI.Units.smallSpacing
//                 spacing: LingmoUI.Units.largeSpacing

//                 Image {
//                     height: 16
//                     width: height
//                     sourceSize: Qt.size(width, height)
//                     source: "qrc:/images/" + (LingmoUI.Theme.darkMode ? "dark" : "light") + "/brightness.svg"
//                     smooth: false
//                     antialiasing: true
//                 }

//                 Timer {
//                     id: brightnessTimer
//                     interval: 100
//                     repeat: false
//                     onTriggered: brightness.setValue(brightnessSlider.value)
//                 }

//                 Slider {
//                     id: brightnessSlider
//                     from: 1
//                     to: 100
//                     stepSize: 1
//                     value: brightness.value
//                     Layout.fillWidth: true
//                     Layout.fillHeight: true
//                     onMoved: brightnessTimer.start()
//                 }

// //                Label {
// //                    text: brightnessSlider.value + "%"
// //                    color: LingmoUI.Theme.disabledTextColor
// //                    Layout.preferredWidth: _fontMetrics.advanceWidth("100%")
// //                }
//             }
//         }

//         Item {
//             id: volumeItem
//             Layout.fillWidth: true
//             height: 40
//             visible: defaultSink

//             Rectangle {
//                 id: volumeItemBg
//                 anchors.fill: parent
//                 color: "white"
//                 radius: LingmoUI.Theme.bigRadius
//                 opacity: LingmoUI.Theme.darkMode ? 0.2 : 0.7
//             }

//             RowLayout {
//                 anchors.fill: volumeItemBg
//                 anchors.leftMargin: LingmoUI.Units.largeSpacing
//                 anchors.rightMargin: LingmoUI.Units.largeSpacing
//                 anchors.topMargin: LingmoUI.Units.smallSpacing
//                 anchors.bottomMargin: LingmoUI.Units.smallSpacing
//                 spacing: LingmoUI.Units.largeSpacing

//                 Image {
//                     height: 16
//                     width: height
//                     sourceSize: Qt.size(width, height)
//                     source: "qrc:/images/" + (LingmoUI.Theme.darkMode ? "dark" : "light") + "/" + control.volumeIconName + ".svg"
//                     smooth: false
//                     antialiasing: true
//                 }

//                 Slider {
//                     id: volumeSlider

//                     Layout.fillWidth: true
//                     Layout.fillHeight: true

//                     from: PulseAudio.MinimalVolume
//                     to: PulseAudio.NormalVolume

//                     stepSize: to / (to / PulseAudio.NormalVolume * 100.0)

//                     value: defaultSink ? defaultSink.volume : 0

//                     onValueChanged: {
//                         if (!defaultSink)
//                             return

//                         defaultSink.volume = value
//                         defaultSink.muted = (value === 0)
//                     }
//                 }

// //                Label {
// //                    text: parseInt(volumeSlider.value / PulseAudio.NormalVolume * 100.0) + "%"
// //                    Layout.preferredWidth: _fontMetrics.advanceWidth("100%")
// //                    color: LingmoUI.Theme.disabledTextColor
// //                }
//             }
//         }

//         FontMetrics {
//             id: _fontMetrics
//         }

//         RowLayout {
//             Layout.leftMargin: LingmoUI.Units.smallSpacing
//             Layout.rightMargin: LingmoUI.Units.smallSpacing
//             spacing: 0

//             Label {
//                 id: timeLabel
//                 leftPadding: LingmoUI.Units.smallSpacing / 2
//                 color: LingmoUI.Theme.textColor

//                 Timer {
//                     interval: 1000
//                     repeat: true
//                     running: true
//                     triggeredOnStart: true
//                     onTriggered: {
//                         timeLabel.text = new Date().toLocaleDateString(Qt.locale(), Locale.LongFormat)
//                     }
//                 }
//             }

//             Item {
//                 Layout.fillWidth: true
//             }

//             StandardItem {
//                 width: batteryLayout.implicitWidth + LingmoUI.Units.largeSpacing
//                 height: batteryLayout.implicitHeight + LingmoUI.Units.largeSpacing

//                 onClicked: {
//                     control.visible = false
//                     process.startDetached("lingmo-settings", ["-m", "battery"])
//                 }

//                 RowLayout {
//                     id: batteryLayout
//                     anchors.fill: parent
//                     visible: battery.available
//                     spacing: 0

//                     Image {
//                         id: batteryIcon
//                         width: 22
//                         height: 16
//                         sourceSize: Qt.size(width, height)
//                         source: "qrc:/images/" + (LingmoUI.Theme.darkMode ? "dark/" : "light/") + battery.iconSource
//                         asynchronous: true
//                         Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
//                         antialiasing: true
//                         smooth: false
//                     }

//                     Label {
//                         text: battery.chargePercent + "%"
//                         color: LingmoUI.Theme.textColor
//                         rightPadding: LingmoUI.Units.smallSpacing / 2
//                         Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
//                     }
//                 }
//             }
//         }
//     }
}
