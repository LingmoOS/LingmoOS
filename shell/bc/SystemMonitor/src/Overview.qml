import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import LingmoUI 1.0 as LingmoUI
import "./components" as Components

ItemPage {
    id: overview
    headerTitle: qsTr("Overview")

    ScrollView {
        id: scrollView
        anchors.fill: parent
        anchors.margins: LingmoUI.Units.largeSpacing
        contentWidth: availableWidth
        ScrollBar.vertical.policy: ScrollBar.AsNeeded

        ColumnLayout {
            width: scrollView.availableWidth
            spacing: LingmoUI.Units.largeSpacing

            // 系统信息卡片
            Components.CardItem {
                Layout.fillWidth: true
                Layout.leftMargin: LingmoUI.Units.largeSpacing
                Layout.rightMargin: LingmoUI.Units.largeSpacing
                Layout.preferredHeight: gridLayout.implicitHeight + LingmoUI.Units.largeSpacing * 7
                title: qsTr("System")

                GridLayout {
                    id: gridLayout
                    anchors.fill: parent
                    anchors.margins: LingmoUI.Units.largeSpacing
                    columns: 2
                    rowSpacing: LingmoUI.Units.largeSpacing * 1.5
                    columnSpacing: LingmoUI.Units.largeSpacing * 2

                    Repeater {
                        model: [
                            { label: qsTr("Hostname"), value: systemInfo.hostname },
                            { label: qsTr("Operating System"), value: systemInfo.osInfo },
                            { label: qsTr("Kernel Version"), value: systemInfo.kernelVersion },
                            { label: qsTr("Desktop Environment"), value: systemInfo.desktopEnvironment },
                            { label: qsTr("CPU"), value: systemInfo.cpuModel }
                        ]

                        delegate: RowLayout {
                            Layout.fillWidth: true
                            spacing: LingmoUI.Units.largeSpacing

                            Label {
                                text: modelData.label + ":"
                                opacity: 0.6
                                font.pointSize: 9
                            }

                            Label {
                                text: modelData.value
                                Layout.fillWidth: true
                                wrapMode: Text.Wrap
                                font.pointSize: 9
                            }
                        }
                    }
                }
            }

            // CPU 和内存使用率卡片
            Components.CardItem {
                Layout.fillWidth: true
                Layout.leftMargin: LingmoUI.Units.largeSpacing
                Layout.rightMargin: LingmoUI.Units.largeSpacing
                Layout.preferredHeight: 220
                title: qsTr("Usage")

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: LingmoUI.Units.largeSpacing
                    spacing: LingmoUI.Units.largeSpacing * 2

                    // CPU 圆形进度条
                    Item {
                        Layout.fillHeight: true
                        Layout.preferredWidth: height

                        Rectangle {
                            anchors.fill: parent
                            radius: width / 2
                            color: Qt.rgba(LingmoUI.Theme.highlightColor.r,
                                          LingmoUI.Theme.highlightColor.g,
                                          LingmoUI.Theme.highlightColor.b, 0.1)

                            Canvas {
                                id: cpuCircle
                                anchors.fill: parent
                                anchors.margins: 15
                                antialiasing: true
                                property real value: systemInfo.cpuUsage / 100
                                property color circleColor: LingmoUI.Theme.highlightColor

                                onPaint: {
                                    var ctx = getContext("2d")
                                    // 清除整个画布
                                    ctx.clearRect(0, 0, width, height)
                                    ctx.save()

                                    var centerX = width / 2
                                    var centerY = height / 2
                                    var radius = Math.min(width, height) / 2
                                    var lineWidth = 10

                                    // 绘制背景圆环
                                    ctx.beginPath()
                                    ctx.arc(centerX, centerY, radius - lineWidth / 2, 0, 2 * Math.PI)
                                    ctx.strokeStyle = Qt.rgba(circleColor.r, circleColor.g, circleColor.b, 0.2)
                                    ctx.lineWidth = lineWidth
                                    ctx.lineCap = "round"
                                    ctx.stroke()

                                    // 绘制进度圆环
                                    ctx.beginPath()
                                    ctx.arc(centerX, centerY, radius - lineWidth / 2, -Math.PI / 2, -Math.PI / 2 + value * 2 * Math.PI)
                                    ctx.strokeStyle = Qt.rgba(circleColor.r, circleColor.g, circleColor.b, 0.9)
                                    ctx.lineWidth = lineWidth
                                    ctx.lineCap = "round"
                                    ctx.stroke()

                                    ctx.restore()
                                }

                                Timer {
                                    interval: systemInfo.updateInterval
                                    running: true
                                    repeat: true
                                    onTriggered: parent.requestPaint()
                                }
                            }

                            ColumnLayout {
                                anchors.centerIn: parent
                                spacing: 2

                                Label {
                                    text: Number(systemInfo.cpuUsage).toFixed(1) + "%"
                                    font.pointSize: 22
                                    font.bold: true
                                    color: LingmoUI.Theme.highlightColor
                                    Layout.alignment: Qt.AlignHCenter
                                }

                                Label {
                                    text: qsTr("CPU")
                                    font.pointSize: 9
                                    opacity: 0.6
                                    Layout.alignment: Qt.AlignHCenter
                                }
                            }
                        }
                    }

                    // 内存圆形进度条
                    Item {
                        Layout.fillHeight: true
                        Layout.preferredWidth: height

                        Rectangle {
                            anchors.fill: parent
                            radius: width / 2
                            color: Qt.rgba(LingmoUI.Theme.highlightColor.r,
                                          LingmoUI.Theme.highlightColor.g,
                                          LingmoUI.Theme.highlightColor.b, 0.1)

                            Canvas {
                                id: memoryCircle
                                anchors.fill: parent
                                anchors.margins: 15
                                antialiasing: true
                                property real value: systemInfo.memoryUsage / 100
                                property color circleColor: LingmoUI.Theme.highlightColor

                                onPaint: {
                                    var ctx = getContext("2d")
                                    // 清除整个画布
                                    ctx.clearRect(0, 0, width, height)
                                    ctx.save()

                                    var centerX = width / 2
                                    var centerY = height / 2
                                    var radius = Math.min(width, height) / 2
                                    var lineWidth = 10

                                    // 绘制背景圆环
                                    ctx.beginPath()
                                    ctx.arc(centerX, centerY, radius - lineWidth / 2, 0, 2 * Math.PI)
                                    ctx.strokeStyle = Qt.rgba(circleColor.r, circleColor.g, circleColor.b, 0.2)
                                    ctx.lineWidth = lineWidth
                                    ctx.lineCap = "round"
                                    ctx.stroke()

                                    // 绘制进度圆环
                                    ctx.beginPath()
                                    ctx.arc(centerX, centerY, radius - lineWidth / 2, -Math.PI / 2, -Math.PI / 2 + value * 2 * Math.PI)
                                    ctx.strokeStyle = Qt.rgba(circleColor.r, circleColor.g, circleColor.b, 0.9)
                                    ctx.lineWidth = lineWidth
                                    ctx.lineCap = "round"
                                    ctx.stroke()

                                    ctx.restore()
                                }

                                Timer {
                                    interval: systemInfo.updateInterval
                                    running: true
                                    repeat: true
                                    onTriggered: parent.requestPaint()
                                }
                            }

                            ColumnLayout {
                                anchors.centerIn: parent
                                spacing: 2

                                Label {
                                    text: Number(systemInfo.memoryUsage).toFixed(1) + "%"
                                    font.pointSize: 22
                                    font.bold: true
                                    color: LingmoUI.Theme.highlightColor
                                    Layout.alignment: Qt.AlignHCenter
                                }

                                Label {
                                    text: qsTr("Memory")
                                    font.pointSize: 9
                                    opacity: 0.6
                                    Layout.alignment: Qt.AlignHCenter
                                }

                                Label {
                                    text: systemInfo.usedMemory + " / " + systemInfo.totalMemory
                                    font.pointSize: 8
                                    opacity: 0.6
                                    Layout.alignment: Qt.AlignHCenter
                                }
                            }
                        }
                    }

                    // 添加磁盘使用情况
                    Item {
                        Layout.fillHeight: true
                        Layout.preferredWidth: height

                        Rectangle {
                            anchors.fill: parent
                            radius: width / 2
                            color: Qt.rgba(LingmoUI.Theme.highlightColor.r,
                                          LingmoUI.Theme.highlightColor.g,
                                          LingmoUI.Theme.highlightColor.b, 0.1)

                            Canvas {
                                id: diskCircle
                                anchors.fill: parent
                                anchors.margins: 15
                                antialiasing: true
                                property real value: {
                                    // 获取第一个磁盘的使用率
                                    if (systemInfo.diskList.length > 0) {
                                        return systemInfo.diskList[0].usedPercentage / 100
                                    }
                                    return 0
                                }
                                property color circleColor: LingmoUI.Theme.highlightColor

                                onPaint: {
                                    var ctx = getContext("2d")
                                    ctx.clearRect(0, 0, width, height)
                                    ctx.save()

                                    var centerX = width / 2
                                    var centerY = height / 2
                                    var radius = Math.min(width, height) / 2
                                    var lineWidth = 10

                                    // 绘制背景圆环
                                    ctx.beginPath()
                                    ctx.arc(centerX, centerY, radius - lineWidth / 2, 0, 2 * Math.PI)
                                    ctx.strokeStyle = Qt.rgba(circleColor.r, circleColor.g, circleColor.b, 0.2)
                                    ctx.lineWidth = lineWidth
                                    ctx.lineCap = "round"
                                    ctx.stroke()

                                    // 绘制进度圆环
                                    ctx.beginPath()
                                    ctx.arc(centerX, centerY, radius - lineWidth / 2, -Math.PI / 2, -Math.PI / 2 + value * 2 * Math.PI)
                                    ctx.strokeStyle = Qt.rgba(circleColor.r, circleColor.g, circleColor.b, 0.9)
                                    ctx.lineWidth = lineWidth
                                    ctx.lineCap = "round"
                                    ctx.stroke()

                                    ctx.restore()
                                }

                                Timer {
                                    interval: systemInfo.updateInterval
                                    running: true
                                    repeat: true
                                    onTriggered: parent.requestPaint()
                                }
                            }

                            ColumnLayout {
                                anchors.centerIn: parent
                                spacing: 2

                                Label {
                                    text: systemInfo.diskList.length > 0 ? 
                                          Number(systemInfo.diskList[0].usedPercentage).toFixed(1) + "%" : "0%"
                                    font.pointSize: 22
                                    font.bold: true
                                    color: LingmoUI.Theme.highlightColor
                                    Layout.alignment: Qt.AlignHCenter
                                }

                                Label {
                                    text: qsTr("Disk")
                                    font.pointSize: 9
                                    opacity: 0.6
                                    Layout.alignment: Qt.AlignHCenter
                                }

                                Label {
                                    text: systemInfo.diskList.length > 0 ? 
                                          systemInfo.diskList[0].free + " " + qsTr("Free") : ""
                                    font.pointSize: 8
                                    opacity: 0.6
                                    Layout.alignment: Qt.AlignHCenter
                                }
                            }
                        }
                    }
                }
            }

            Item {
                Layout.fillHeight: true
            }
        }
    }
} 