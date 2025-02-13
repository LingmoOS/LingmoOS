import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import LingmoUI 1.0 as LingmoUI
import "./components" as Components

ItemPage {
    id: resources
    headerTitle: qsTr("Resources")

    // 添加属性来存储历史数据
    property var cpuHistory: []
    property var memoryHistory: []
    property var networkDownloadHistory: []
    property var networkUploadHistory: []

    // 在组件完成时初始化数据
    Component.onCompleted: {
        // 初始化 CPU 历史数据
        if (cpuHistory.length === 0) {
            cpuHistory = new Array(cpuCanvas.maxPoints).fill(0)
        }
        cpuCanvas.values = cpuHistory

        // 初始化内存历史数据
        if (memoryHistory.length === 0) {
            memoryHistory = new Array(memoryCanvas.maxPoints).fill(0)
        }
        memoryCanvas.values = memoryHistory

        // 初始化网络历史数据
        if (networkDownloadHistory.length === 0) {
            networkDownloadHistory = new Array(networkCanvas.maxPoints).fill(0)
        }
        if (networkUploadHistory.length === 0) {
            networkUploadHistory = new Array(networkCanvas.maxPoints).fill(0)
        }
        networkCanvas.downloadValues = networkDownloadHistory
        networkCanvas.uploadValues = networkUploadHistory
    }

    // 在组件销毁前保存数据
    Component.onDestruction: {
        cpuHistory = cpuCanvas.values
        memoryHistory = memoryCanvas.values
        networkDownloadHistory = networkCanvas.downloadValues
        networkUploadHistory = networkCanvas.uploadValues
    }

    ScrollView {
        id: scrollView
        anchors.fill: parent
        anchors.margins: LingmoUI.Units.largeSpacing
        contentWidth: availableWidth
        ScrollBar.vertical.policy: ScrollBar.AsNeeded

        ColumnLayout {
            width: scrollView.availableWidth
            spacing: LingmoUI.Units.largeSpacing

            // CPU History 卡片
            Components.CardItem {
                Layout.fillWidth: true
                Layout.leftMargin: LingmoUI.Units.largeSpacing
                Layout.rightMargin: LingmoUI.Units.largeSpacing
                Layout.preferredHeight: 200 + cpuCoresGrid.height
                title: qsTr("CPU History")

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: LingmoUI.Units.largeSpacing
                    spacing: LingmoUI.Units.mediumSpacing

                    RowLayout {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 3
                        Layout.topMargin: -LingmoUI.Units.smallSpacing
                        spacing: LingmoUI.Units.largeSpacing

                        Label {
                            text: Number(systemInfo.cpuUsage).toFixed(1) + "%"
                            font.pointSize: 16
                            color: LingmoUI.Theme.highlightColor
                        }

                        Components.ProgressBar {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 6
                            Layout.alignment: Qt.AlignVCenter
                            from: 0
                            to: 100
                            value: systemInfo.cpuUsage
                        }
                    }

                    // 添加 CPU 核心网格
                    GridLayout {
                        id: cpuCoresGrid
                        Layout.fillWidth: true
                        Layout.topMargin: LingmoUI.Units.smallSpacing
                        columns: Math.max(2, Math.min(4, systemInfo.cpuCoreCount))  // 根据核心数量自动调整列数
                        rowSpacing: LingmoUI.Units.smallSpacing
                        columnSpacing: LingmoUI.Units.smallSpacing

                        Repeater {
                            model: systemInfo.cpuCoreCount

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: LingmoUI.Units.smallSpacing

                                Label {
                                    text: qsTr("Core %1:").arg(modelData + 1)
                                    opacity: 0.7
                                    Layout.preferredWidth: 60
                                }

                                Components.ProgressBar {
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: 4  // 稍微减小高度
                                    from: 0
                                    to: 100
                                    value: systemInfo.cpuCoreUsage[modelData]
                                }

                                Label {
                                    text: Number(systemInfo.cpuCoreUsage[modelData]).toFixed(1) + "%"
                                    opacity: 0.7
                                    Layout.preferredWidth: 50
                                    horizontalAlignment: Text.AlignRight
                                }
                            }
                        }
                    }

                    Canvas {
                        id: cpuCanvas
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        Layout.topMargin: LingmoUI.Units.smallSpacing
                        property var values: []
                        property int maxPoints: width / 2
                        property real currentValue: 0
                        property real targetValue: 0
                        
                        NumberAnimation {
                            id: cpuValueAnimation
                            target: cpuCanvas
                            property: "currentValue"
                            duration: 300
                            easing.type: Easing.OutCubic
                        }

                        onWidthChanged: {
                            adjustPoints()
                            requestPaint()
                        }

                        function adjustPoints() {
                            var newMaxPoints = width / 2
                            if (newMaxPoints === maxPoints)
                                return

                            var newValues = []
                            if (newMaxPoints > maxPoints) {
                                var diff = newMaxPoints - maxPoints
                                for (var i = 0; i < diff; i++) {
                                    newValues.push(0)
                                }
                                newValues = newValues.concat(values)
                            } else {
                                newValues = values.slice(-newMaxPoints)
                            }
                            
                            values = newValues
                            maxPoints = newMaxPoints
                        }

                        onPaint: {
                            var ctx = getContext("2d")
                            ctx.clearRect(0, 0, width, height)

                            // 保存原始状态
                            ctx.save()
                            
                            // 调整边距，为右侧也预留空间
                            var leftMargin = 30
                            var rightMargin = 20  // 添加右边距
                            var bottomMargin = 20
                            var topMargin = 10
                            var graphWidth = width - leftMargin - rightMargin
                            var graphHeight = height - bottomMargin - topMargin

                            // 绘制网格和标注
                            ctx.strokeStyle = LingmoUI.Theme.textColor
                            ctx.fillStyle = LingmoUI.Theme.textColor
                            ctx.globalAlpha = 0.1
                            ctx.lineWidth = 1
                            ctx.font = "10px sans-serif"
                            ctx.textAlign = "right"
                            ctx.textBaseline = "middle"

                            // 横向网格和百分比标注
                            for (var i = 0; i <= 4; i++) {
                                var y = (graphHeight / 4) * i
                                var value = 100 - (i * 25)
                                
                                // 绘制网格线，注意结束位置
                                ctx.beginPath()
                                ctx.moveTo(leftMargin, y)
                                ctx.lineTo(width - rightMargin, y)  // 修改结束位置
                                
                                // 绘制百分比标注
                                if (value === 100) {
                                    ctx.lineWidth = 2; // 设置粗线宽
                                } else {
                                    ctx.lineWidth = 1; // 恢复默认线宽
                                }
                                ctx.stroke()
                                
                                // 只绘制非100%的百分比标注
                                if (value !== 100) {
                                    ctx.globalAlpha = 0.7
                                    ctx.fillText(value + "%", leftMargin - 5, y)
                                    ctx.globalAlpha = 0.1
                                }
                            }

                            // 纵向网格和时间标注
                            var timeInterval = 10  // 每个格子代表的秒数
                            ctx.textAlign = "center"
                            ctx.textBaseline = "top"
                            
                            for (var j = 0; j <= 6; j++) {
                                var x = leftMargin + (graphWidth / 6) * j
                                var timeValue = -timeInterval * (6 - j)
                                
                                // 绘制网格线
                                ctx.beginPath()
                                ctx.moveTo(x, 0)
                                ctx.lineTo(x, graphHeight)
                                ctx.stroke()
                                
                                // 绘制时间标注
                                ctx.globalAlpha = 0.7
                                ctx.fillText(timeValue + "s", x, graphHeight + 5)
                                ctx.globalAlpha = 0.1
                            }

                            // 恢复原始状态
                            ctx.restore()

                            if (values.length < 2) return

                            // 绘制曲线
                            ctx.globalAlpha = 1.0
                            ctx.beginPath()
                            ctx.strokeStyle = LingmoUI.Theme.highlightColor
                            ctx.lineWidth = 2

                            var stepX = graphWidth / (maxPoints - 1)
                            var stepY = graphHeight / 100

                            // 创建渐变填充
                            var gradient = ctx.createLinearGradient(0, 0, 0, graphHeight)
                            gradient.addColorStop(0, Qt.rgba(LingmoUI.Theme.highlightColor.r,
                                                           LingmoUI.Theme.highlightColor.g,
                                                           LingmoUI.Theme.highlightColor.b, 0.3))
                            gradient.addColorStop(1, Qt.rgba(LingmoUI.Theme.highlightColor.r,
                                                           LingmoUI.Theme.highlightColor.g,
                                                           LingmoUI.Theme.highlightColor.b, 0))

                            // 绘制曲线和填充
                            ctx.moveTo(leftMargin, graphHeight - values[0] * stepY)
                            for (var k = 1; k < values.length; k++) {
                                ctx.lineTo(leftMargin + k * stepX, graphHeight - values[k] * stepY)
                            }
                            ctx.stroke()

                            // 修改曲线绘制的结束位置
                            ctx.lineTo(width - rightMargin, graphHeight)
                            ctx.lineTo(leftMargin, graphHeight)
                            ctx.closePath()
                            ctx.fillStyle = gradient
                            ctx.fill()
                        }

                        Timer {
                            interval: systemInfo.updateInterval
                            running: true
                            repeat: true
                            onTriggered: {
                                var values = cpuCanvas.values
                                cpuCanvas.targetValue = systemInfo.cpuUsage
                                cpuValueAnimation.from = cpuCanvas.currentValue
                                cpuValueAnimation.to = cpuCanvas.targetValue
                                cpuValueAnimation.start()
                                
                                values.push(cpuCanvas.targetValue)
                                if (values.length > cpuCanvas.maxPoints)
                                    values.shift()
                                cpuCanvas.values = values
                                cpuHistory = values
                                cpuCanvas.requestPaint()
                            }
                        }
                    }
                }
            }

            // Memory History 卡片
            Components.CardItem {
                Layout.fillWidth: true
                Layout.leftMargin: LingmoUI.Units.largeSpacing
                Layout.rightMargin: LingmoUI.Units.largeSpacing
                Layout.preferredHeight: 230
                title: qsTr("Memory History")

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: LingmoUI.Units.largeSpacing
                    spacing: LingmoUI.Units.mediumSpacing

                    RowLayout {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 3
                        spacing: LingmoUI.Units.largeSpacing

                        ColumnLayout {
                            spacing: 0
                            Layout.alignment: Qt.AlignVCenter

                            Label {
                                text: systemInfo.usedMemory
                                font.pointSize: 16
                                color: LingmoUI.Theme.highlightColor
                            }

                            Label {
                                text: qsTr("of %1").arg(systemInfo.totalMemory)
                                opacity: 0.7
                                font.pointSize: 8
                            }
                        }

                        Components.ProgressBar {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 6
                            Layout.alignment: Qt.AlignVCenter
                            from: 0
                            to: 100
                            value: systemInfo.memoryUsage
                        }
                    }

                    Item {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 10
                    }

                    Canvas {
                        id: memoryCanvas
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        Layout.topMargin: LingmoUI.Units.smallSpacing
                        property var values: []
                        property int maxPoints: width / 2
                        property real currentValue: 0
                        property real targetValue: 0

                        NumberAnimation {
                            id: memoryValueAnimation
                            target: memoryCanvas
                            property: "currentValue"
                            duration: 300
                            easing.type: Easing.OutCubic
                        }

                        onWidthChanged: {
                            adjustPoints()
                            requestPaint()
                        }

                        function adjustPoints() {
                            var newMaxPoints = width / 2
                            if (newMaxPoints === maxPoints)
                                return

                            var newValues = []
                            if (newMaxPoints > maxPoints) {
                                var diff = newMaxPoints - maxPoints
                                for (var i = 0; i < diff; i++) {
                                    newValues.push(0)
                                }
                                newValues = newValues.concat(values)
                            } else {
                                newValues = values.slice(-newMaxPoints)
                            }
                            
                            values = newValues
                            maxPoints = newMaxPoints
                        }

                        onPaint: {
                            var ctx = getContext("2d")
                            ctx.clearRect(0, 0, width, height)

                            // 保存原始状态
                            ctx.save()
                            
                            // 调整边距，为右侧也预留空间
                            var leftMargin = 30
                            var rightMargin = 20  // 添加右边距
                            var bottomMargin = 20
                            var topMargin = 10
                            var graphWidth = width - leftMargin - rightMargin
                            var graphHeight = height - bottomMargin - topMargin

                            // 绘制网格和标注
                            // ctx.strokeStyle = LingmoUI.Theme.textColor
                            ctx.fillStyle = LingmoUI.Theme.textColor
                            ctx.globalAlpha = 0.1
                            ctx.lineWidth = 1
                            ctx.font = "10px sans-serif"
                            ctx.textAlign = "right"
                            ctx.textBaseline = "middle"

                            // 横向网格和百分比标注
                            for (var i = 0; i <= 4; i++) {
                                var y = (graphHeight / 4) * i
                                var value = 100 - (i * 25)
                                
                                // 绘制网格线，注意结束位置
                                ctx.beginPath()
                                ctx.moveTo(leftMargin, y)
                                ctx.lineTo(width - rightMargin, y)  // 修改结束位置

                                if (value === 100) {
                                    ctx.lineWidth = 2; // 设置粗线宽
                                } else {
                                    ctx.lineWidth = 1; // 恢复默认线宽
                                }
                                ctx.stroke()

                                if (value !== 100) {
                                    ctx.globalAlpha = 0.7
                                    ctx.fillText(value + "%", leftMargin - 5, y)
                                    ctx.globalAlpha = 0.1
                                }
                            }

                            // 纵向网格和时间标注
                            var timeInterval = 10  // 每个格子代表的秒数
                            ctx.textAlign = "center"
                            ctx.textBaseline = "top"
                            
                            for (var j = 0; j <= 6; j++) {
                                var x = leftMargin + (graphWidth / 6) * j
                                var timeValue = -timeInterval * (6 - j)
                                
                                // 绘制网格线
                                ctx.beginPath()
                                ctx.moveTo(x, 0)
                                ctx.lineTo(x, graphHeight)
                                ctx.stroke()
                                
                                // 绘制时间标注
                                ctx.globalAlpha = 0.7
                                ctx.fillText(timeValue + "s", x, graphHeight + 5)
                                ctx.globalAlpha = 0.1
                            }

                            // 恢复原始状态
                            ctx.restore()

                            if (values.length < 2) return

                            // 绘制曲线
                            ctx.globalAlpha = 1.0
                            ctx.beginPath()
                            ctx.strokeStyle = LingmoUI.Theme.highlightColor
                            ctx.lineWidth = 2

                            var stepX = graphWidth / (maxPoints - 1)
                            var stepY = graphHeight / 100

                            // 创建渐变填充
                            var gradient = ctx.createLinearGradient(0, 0, 0, graphHeight)
                            gradient.addColorStop(0, Qt.rgba(LingmoUI.Theme.highlightColor.r,
                                                           LingmoUI.Theme.highlightColor.g,
                                                           LingmoUI.Theme.highlightColor.b, 0.3))
                            gradient.addColorStop(1, Qt.rgba(LingmoUI.Theme.highlightColor.r,
                                                           LingmoUI.Theme.highlightColor.g,
                                                           LingmoUI.Theme.highlightColor.b, 0))

                            // 绘制曲线和填充
                            ctx.moveTo(leftMargin, graphHeight - values[0] * stepY + topMargin)
                            for (var k = 1; k < values.length; k++) {
                                ctx.lineTo(leftMargin + k * stepX, graphHeight - values[k] * stepY + topMargin)
                            }
                            ctx.stroke()

                            // 修改曲线绘制的结束位置
                            ctx.lineTo(width - rightMargin, graphHeight)
                            ctx.lineTo(leftMargin, graphHeight)
                            ctx.closePath()
                            ctx.fillStyle = gradient
                            ctx.fill()
                        }

                        Timer {
                            interval: systemInfo.updateInterval
                            running: true
                            repeat: true
                            onTriggered: {
                                var values = memoryCanvas.values
                                memoryCanvas.targetValue = systemInfo.memoryUsage
                                memoryValueAnimation.from = memoryCanvas.currentValue
                                memoryValueAnimation.to = memoryCanvas.targetValue
                                memoryValueAnimation.start()
                                
                                values.push(memoryCanvas.targetValue)
                                if (values.length > memoryCanvas.maxPoints)
                                    values.shift()
                                memoryCanvas.values = values
                                memoryHistory = values
                                memoryCanvas.requestPaint()
                            }
                        }
                    }
                }
            }

            // Network History 卡片
            Components.CardItem {
                Layout.fillWidth: true
                Layout.leftMargin: LingmoUI.Units.largeSpacing
                Layout.rightMargin: LingmoUI.Units.largeSpacing
                Layout.preferredHeight: 300
                title: qsTr("Network History")

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: LingmoUI.Units.largeSpacing
                    spacing: LingmoUI.Units.mediumSpacing

                    // 添加表格头部
                    Rectangle {
                        Layout.fillWidth: true
                        height: 36
                        color: LingmoUI.Theme.darkMode ? Qt.rgba(255, 255, 255, 0.1)
                                                      : Qt.rgba(0, 0, 0, 0.05)
                        radius: LingmoUI.Theme.smallRadius

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: LingmoUI.Units.largeSpacing
                            anchors.rightMargin: LingmoUI.Units.largeSpacing
                            spacing: LingmoUI.Units.largeSpacing

                            Label {
                                text: qsTr("Interface")
                                Layout.preferredWidth: 120
                                opacity: 0.7
                            }

                            Label {
                                text: qsTr("Download")
                                Layout.preferredWidth: 100
                                opacity: 0.7
                            }

                            Label {
                                text: qsTr("Upload")
                                Layout.preferredWidth: 100
                                opacity: 0.7
                            }

                            Label {
                                text: qsTr("Total")
                                Layout.fillWidth: true
                                opacity: 0.7
                            }
                        }
                    }

                    // 网络接口列表
                    ListView {
                        id: networkListView
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        model: systemInfo.networkList
                        spacing: LingmoUI.Units.smallSpacing
                        clip: true

                        delegate: Rectangle {
                            width: ListView.view.width
                            height: 36
                            color: index % 2 ? (LingmoUI.Theme.darkMode ? Qt.rgba(255, 255, 255, 0.05)
                                                                       : Qt.rgba(0, 0, 0, 0.03))
                                                    : "transparent"

                            RowLayout {
                                anchors.fill: parent
                                anchors.leftMargin: LingmoUI.Units.largeSpacing
                                anchors.rightMargin: LingmoUI.Units.largeSpacing
                                spacing: LingmoUI.Units.largeSpacing

                                Label {
                                    text: modelData.interface
                                    Layout.preferredWidth: 120
                                    elide: Text.ElideRight
                                }

                                Label {
                                    text: modelData.download
                                    Layout.preferredWidth: 100
                                    color: LingmoUI.Theme.highlightColor
                                }

                                Label {
                                    text: modelData.upload
                                    Layout.preferredWidth: 100
                                    color: LingmoUI.Theme.highlightColor
                                }

                                Label {
                                    text: modelData.total || "0 B"  // 需要在 systeminfo.cpp 中添加
                                    Layout.fillWidth: true
                                    color: LingmoUI.Theme.textColor
                                }
                            }
                        }

                        // 添加空状态提示
                        Label {
                            anchors.centerIn: parent
                            text: qsTr("No network interfaces found")
                            visible: networkListView.count === 0
                            opacity: 0.5
                        }
                    }
                }
            }

            // 底部间距
            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: LingmoUI.Units.largeSpacing
            }
        }
    }
} 