import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtGraphicalEffects 1.0
import LingmoUI 1.0 as LingmoUI

ItemPage {
    id: stopwatchView
    headerTitle: qsTr("Stopwatch")
    property int milliseconds: 0
    property bool running: false

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: LingmoUI.Units.largeSpacing
        spacing: LingmoUI.Units.largeSpacing

        // 上部分：数字显示和控制按钮
        RowLayout {
            Layout.fillWidth: true
            spacing: LingmoUI.Units.largeSpacing

            // 数字显示
            Label {
                id: stopwatchDisplay
                text: formatTime(milliseconds)
                font {
                    pixelSize: 32
                    family: "Monospace"
                    bold: true
                }
                Layout.alignment: Qt.AlignVCenter
                color: LingmoUI.Theme.textColor
            }

            // 控制按钮
            Row {
                Layout.alignment: Qt.AlignVCenter
                spacing: LingmoUI.Units.smallSpacing

                Button {
                    text: running ? qsTr("暂停") : qsTr("开始")
                    icon.name: running ? "media-playback-pause" : "media-playback-start"
                    highlighted: !running
                    onClicked: {
                        running = !running
                        timer.running = running
                    }
                }

                Button {
                    text: qsTr("计次")
                    icon.name: "list-add"
                    enabled: running
                    onClicked: {
                        lapAnimation.start()
                        lapModel.insert(0, {
                            "lapTime": formatTime(milliseconds),
                            "splitTime": formatTime(milliseconds - (lapModel.count > 0 ? 
                                parseTime(lapModel.get(0).lapTime) : 0)),
                            "handRotation": secondHand.rotation  // 保存当前秒针角度
                        })
                    }
                }

                Button {
                    text: qsTr("重置")
                    icon.name: "media-playback-stop"
                    onClicked: {
                        running = false
                        timer.running = false
                        
                        // 先执行指针动画，再清零
                        resetAnimation.start()
                    }
                }
            }

            Item {
                Layout.fillWidth: true
            }
        }

        // 下部分：模拟秒表和计次列表
        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: LingmoUI.Units.largeSpacing

            // 左侧模拟秒表
            Rectangle {
                Layout.fillHeight: true
                Layout.preferredWidth: height
                Layout.maximumWidth: 300
                Layout.maximumHeight: 300
                color: "transparent"
                
                Rectangle {
                    anchors.fill: parent
                    radius: width / 2
                    color: "transparent"
                    border.width: 2
                    border.color: LingmoUI.Theme.darkMode ? 
                        Qt.rgba(1, 1, 1, 0.1) : Qt.rgba(0, 0, 0, 0.1)
                }

                Rectangle {
                    anchors.fill: parent
                    anchors.margins: 2
                    radius: width / 2
                    color: LingmoUI.Theme.backgroundColor
                    opacity: LingmoUI.Theme.darkMode ? 0.2 : 0.7

                    layer.enabled: true
                    layer.effect: FastBlur {
                        radius: 32
                    }

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

                    Rectangle {
                        anchors.fill: parent
                        radius: parent.radius
                        color: "transparent"
                        border.width: 1
                        border.color: LingmoUI.Theme.darkMode ? 
                            Qt.rgba(1, 1, 1, 0.1) : Qt.rgba(1, 1, 1, 0.3)
                    }
                }

                // 秒表内容
                Item {
                    id: stopwatch
                    anchors.centerIn: parent
                    width: Math.min(parent.width, parent.height) - 44
                    height: width

                    // 数字时间显示
                    Label {
                        anchors.centerIn: parent
                        anchors.verticalCenterOffset: -stopwatch.height * 0.15
                        text: formatTime(milliseconds)
                        font {
                            pixelSize: stopwatch.width * 0.12
                            family: "Monospace"
                            bold: true
                        }
                        color: LingmoUI.Theme.textColor
                    }

                    // 毫秒显示
                    Label {
                        anchors.centerIn: parent
                        anchors.verticalCenterOffset: stopwatch.height * 0.15
                        text: (milliseconds % 1000).toString().padStart(3, '0')
                        font {
                            pixelSize: stopwatch.width * 0.06
                            family: "Monospace"
                        }
                        color: LingmoUI.Theme.textColor
                        opacity: 0.7
                    }

                    // 刻度
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
                                ctx.globalAlpha = isHour ? 0.8 : 0.3;
                                
                                var outerRadius = radius;
                                var innerRadius = radius - (isHour ? radius * 0.08 : radius * 0.04);
                                
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

                            ctx.restore();
                        }
                    }

                    // 数字刻度（只显示主要刻度）
                    Repeater {
                        model: 12
                        
                        Label {
                            property real angle: (index * 30) * Math.PI / 180
                            property real radius: stopwatch.width * 0.43
                            
                            x: stopwatch.width / 2 - width / 2 + Math.sin(angle) * radius
                            y: stopwatch.height / 2 - height / 2 - Math.cos(angle) * radius
                            text: index === 0 ? "60" : (index * 5).toString()
                            font {
                                pixelSize: stopwatch.width * 0.05
                                family: "Monospace"
                                bold: true
                            }
                            color: LingmoUI.Theme.textColor
                            opacity: 0.9
                        }
                    }

                    // 计次指针
                    Repeater {
                        id: lapRepeater
                        model: lapModel
                        
                        Rectangle {
                            width: stopwatch.width * 0.015
                            height: stopwatch.height * 0.45
                            color: LingmoUI.Theme.highlightColor
                            opacity: 0.3
                            radius: width / 2
                            anchors.bottom: parent.verticalCenter
                            anchors.horizontalCenter: parent.horizontalCenter
                            transformOrigin: Item.Bottom
                            antialiasing: true
                            rotation: model.handRotation
                            z: 1

                            layer.enabled: true
                            layer.effect: DropShadow {
                                radius: 4
                                samples: 8
                                color: Qt.rgba(0, 0, 0, 0.3)
                            }

                            Behavior on rotation {
                                NumberAnimation {
                                    duration: 200
                                    easing.type: Easing.OutCubic
                                }
                            }
                        }
                    }

                    // 秒针
                    Rectangle {
                        id: secondHand
                        width: stopwatch.width * 0.015
                        height: stopwatch.height * 0.45
                        color: LingmoUI.Theme.highlightColor
                        radius: width / 2
                        anchors.bottom: parent.verticalCenter
                        anchors.horizontalCenter: parent.horizontalCenter
                        transformOrigin: Item.Bottom
                        antialiasing: true
                        rotation: (milliseconds / 1000) * 6
                        z: 2

                        layer.enabled: true
                        layer.effect: DropShadow {
                            radius: 4
                            samples: 8
                            color: LingmoUI.Theme.darkMode ? 
                                Qt.rgba(0, 0, 0, 0.5) : Qt.rgba(0, 0, 0, 0.3)
                        }
                    }

                    // 独立的中心点
                    Rectangle {
                        width: stopwatch.width * 0.06  // 调整大小
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

            // 右侧计次列表
            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                color: LingmoUI.Theme.darkMode ? 
                    Qt.rgba(0, 0, 0, 0.2) : Qt.rgba(0, 0, 0, 0.05)
                radius: LingmoUI.Theme.mediumRadius

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: LingmoUI.Units.largeSpacing
                    spacing: 0

                    // 表头
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: LingmoUI.Units.largeSpacing

                        Label {
                            text: qsTr("序号")
                            Layout.preferredWidth: 60
                            color: LingmoUI.Theme.textColor
                            opacity: 0.5
                        }
                        Label {
                            text: qsTr("计次时间")
                            Layout.preferredWidth: 100
                            color: LingmoUI.Theme.textColor
                            opacity: 0.5
                        }
                        Label {
                            text: qsTr("分段时间")
                            Layout.fillWidth: true
                            color: LingmoUI.Theme.textColor
                            opacity: 0.5
                        }
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        height: 1
                        color: LingmoUI.Theme.textColor
                        opacity: 0.1
                        Layout.topMargin: LingmoUI.Units.smallSpacing
                        Layout.bottomMargin: LingmoUI.Units.smallSpacing
                    }

                    ListView {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        model: ListModel { id: lapModel }
                        clip: true

                        delegate: ItemDelegate {
                            width: parent.width
                            height: 36

                            Rectangle {
                                anchors.fill: parent
                                color: LingmoUI.Theme.textColor
                                opacity: 0.05
                                visible: index % 2 === 0
                            }

                            RowLayout {
                                anchors.fill: parent
                                anchors.margins: LingmoUI.Units.smallSpacing
                                spacing: LingmoUI.Units.largeSpacing

                                Label {
                                    text: qsTr("计次 %1").arg(lapModel.count - index)
                                    Layout.preferredWidth: 60
                                    color: LingmoUI.Theme.textColor
                                }
                                Label {
                                    text: lapTime
                                    Layout.preferredWidth: 100
                                    font.family: "Monospace"
                                    color: LingmoUI.Theme.textColor
                                }
                                Label {
                                    text: splitTime
                                    Layout.fillWidth: true
                                    font.family: "Monospace"
                                    color: LingmoUI.Theme.textColor
                                }
                            }
                        }

                        ScrollBar.vertical: ScrollBar {}
                    }
                }
            }
        }
    }

    Timer {
        id: timer
        interval: 10
        repeat: true
        onTriggered: {
            milliseconds += 10
        }
    }

    function formatTime(ms) {
        var minutes = Math.floor(ms / 60000)
        var seconds = Math.floor((ms % 60000) / 1000)
        var millis = Math.floor((ms % 1000) / 10)
        return minutes.toString().padStart(2, '0') + ":" +
               seconds.toString().padStart(2, '0') + "." +
               millis.toString().padStart(2, '0')
    }

    function parseTime(timeStr) {
        var parts = timeStr.split(/[:.]/);
        return parseInt(parts[0]) * 60000 + 
               parseInt(parts[1]) * 1000 + 
               parseInt(parts[2]) * 10;
    }

    // 添加计次动画
    SequentialAnimation {
        id: lapAnimation
        
        PropertyAnimation {
            target: secondHand
            property: "scale"
            from: 1
            to: 1.2
            duration: 100
        }
        
        PropertyAnimation {
            target: secondHand
            property: "scale"
            from: 1.2
            to: 1
            duration: 100
        }
    }

    // 修改重置动画序列
    SequentialAnimation {
        id: resetAnimation
        
        // 所有指针旋转到零点
        PropertyAction {
            target: secondHand
            property: "running"
            value: false
        }
        
        ParallelAnimation {
            // 秒针旋转动画
            NumberAnimation {
                target: secondHand
                property: "rotation"
                to: 0
                duration: 500
                easing.type: Easing.OutCubic
            }
            
            // 计次指针动画
            ScriptAction {
                script: {
                    // 为每个计次指针添加回零动画
                    for (var i = 0; i < lapRepeater.count; i++) {
                        var lapHand = lapRepeater.itemAt(i)
                        if (lapHand) {
                            lapHand.rotation = 0  // 触发已定义的 Behavior 动画
                        }
                    }
                }
            }
        }
        
        // 等待指针动画完成
        PauseAnimation {
            duration: 500
        }
        
        // 计次指针淡出动画
        NumberAnimation {
            target: lapRepeater
            property: "opacity"
            to: 0
            duration: 300
            easing.type: Easing.InQuad
        }
        
        // 动画完成后清零
        ScriptAction {
            script: {
                milliseconds = 0
                lapModel.clear()
                lapRepeater.opacity = 1  // 重置透明度
            }
        }
    }
} 