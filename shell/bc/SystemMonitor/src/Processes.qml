import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtGraphicalEffects 1.0
import LingmoUI 1.0 as LingmoUI
import "./components" as Components

ItemPage {
    id: processes
    headerTitle: qsTr("Processes")

    property string sortBy: "cpu"
    property bool sortAscending: false
    property string selectedPid: ""
    property var activeContextMenu: null  // 添加这个属性来跟踪活动的菜单

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: LingmoUI.Units.largeSpacing
        spacing: LingmoUI.Units.largeSpacing

        // 搜索和过滤栏
        Components.CardItem {
            Layout.fillWidth: true
            Layout.preferredHeight: 80

            RowLayout {
                anchors.fill: parent
                anchors.margins: LingmoUI.Units.smallSpacing
                spacing: LingmoUI.Units.largeSpacing

                TextField {
                    id: searchField
                    Layout.fillWidth: true
                    Layout.preferredHeight: 36
                    placeholderText: qsTr("Search Process")
                    verticalAlignment: TextInput.AlignVCenter
                    selectByMouse: true
                    
                    // 添加清除按钮
                    rightPadding: clearButton.width + LingmoUI.Units.smallSpacing

                    Button {
                        id: clearButton
                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        width: height - 2
                        height: parent.height - 2
                        visible: searchField.text.length > 0
                        
                        icon.name: "edit-clear"
                        icon.width: 16
                        icon.height: 16
                        
                        background: Rectangle {
                            radius: height / 2
                            color: parent.pressed ? Qt.rgba(0, 0, 0, 0.1) :
                                                   parent.hovered ? Qt.rgba(0, 0, 0, 0.05) : "transparent"
                        }
                        
                        onClicked: {
                            searchField.clear()
                            searchField.forceActiveFocus()
                        }
                    }

                    onTextChanged: {
                        searchTimer.restart()
                    }

                    // 添加快捷键提示
                    ToolTip.visible: hovered
                    ToolTip.text: qsTr("Press Ctrl+F to search")

                    background: Rectangle {
                        implicitWidth: 200
                        implicitHeight: 36
                        color: LingmoUI.Theme.darkMode ? Qt.rgba(255, 255, 255, 0.1)
                                                     : Qt.rgba(0, 0, 0, 0.05)
                        radius: LingmoUI.Theme.smallRadius
                        
                        Rectangle {
                            anchors.fill: parent
                            radius: parent.radius
                            color: "transparent"
                            border.width: 1
                            border.color: parent.parent.activeFocus ? 
                                LingmoUI.Theme.highlightColor : "transparent"
                            opacity: 0.5
                        }
                    }
                }

                ComboBox {
                    id: filterBox
                    Layout.preferredWidth: 150
                    Layout.preferredHeight: 36
                    model: [qsTr("All Processes"), qsTr("My Processes"), qsTr("Active Processes")]
                    onCurrentIndexChanged: updateProcessList()
                    
                    // 添加这些属性来统一高度
                    topInset: 0
                    bottomInset: 0
                    padding: 0
                    
                    background: Rectangle {
                        implicitWidth: 120
                        implicitHeight: 36
                        color: LingmoUI.Theme.darkMode ? Qt.rgba(255, 255, 255, 0.1)
                                                     : Qt.rgba(0, 0, 0, 0.05)
                        radius: LingmoUI.Theme.smallRadius
                        
                        Rectangle {
                            anchors.fill: parent
                            radius: parent.radius
                            color: "transparent"
                            border.width: 1
                            border.color: parent.parent.down ? 
                                LingmoUI.Theme.highlightColor : "transparent"
                            opacity: 0.5
                        }
                    }

                    contentItem: Text {
                        leftPadding: LingmoUI.Units.smallSpacing
                        rightPadding: filterBox.indicator.width + LingmoUI.Units.smallSpacing
                        text: filterBox.displayText
                        font: filterBox.font
                        color: LingmoUI.Theme.textColor
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignLeft
                        elide: Text.ElideRight
                    }

                    indicator: Image {
                        x: filterBox.width - width - LingmoUI.Units.smallSpacing
                        y: (filterBox.height - height) / 2
                        width: 12
                        height: 12
                        source: "qrc:/images/go-down.svg"
                        sourceSize.width: width
                        sourceSize.height: height
                        opacity: enabled ? 1.0 : 0.3
                    }

                    popup: Popup {
                        y: filterBox.height + LingmoUI.Units.smallSpacing
                        width: filterBox.width
                        padding: 1

                        background: Rectangle {
                            color: LingmoUI.Theme.darkMode ? "#363636" : "#FFFFFF"
                            radius: LingmoUI.Theme.smallRadius

                            // 添加模糊背景
                            Rectangle {
                                id: blur
                                anchors.fill: parent
                                radius: parent.radius
                                color: "transparent"

                                layer.enabled: true
                                layer.effect: LingmoUI.WindowBlur {
                                    view: rootWindow
                                    geometry: Qt.rect(blur.x, blur.y, blur.width, blur.height)
                                    windowRadius: blur.radius
                                    enabled: true
                                }
                            }

                            // 添加阴影
                            layer.enabled: true
                            layer.effect: DropShadow {
                                transparentBorder: true
                                radius: 8
                                samples: 16
                                horizontalOffset: 0
                                verticalOffset: 0
                                color: Qt.rgba(0, 0, 0, 0.2)
                            }
                        }

                        contentItem: ListView {
                            clip: true
                            implicitHeight: Math.min(contentHeight, 200)  // 限制最大高度
                            model: filterBox.popup.visible ? filterBox.delegateModel : null
                            currentIndex: filterBox.highlightedIndex
                            spacing: LingmoUI.Units.smallSpacing
                            ScrollIndicator.vertical: ScrollIndicator { }

                            // 添加淡入淡出动画
                            add: Transition {
                                NumberAnimation { property: "opacity"; from: 0; to: 1.0; duration: 100 }
                            }
                            remove: Transition {
                                NumberAnimation { property: "opacity"; from: 1.0; to: 0; duration: 100 }
                            }
                            displaced: Transition {
                                NumberAnimation { properties: "x,y"; duration: 100 }
                            }
                        }

                        // 添加打开/关闭动画
                        enter: Transition {
                            NumberAnimation { property: "opacity"; from: 0.0; to: 1.0; duration: 100 }
                            NumberAnimation { property: "scale"; from: 0.95; to: 1.0; duration: 100 }
                        }
                        
                        exit: Transition {
                            NumberAnimation { property: "opacity"; from: 1.0; to: 0.0; duration: 100 }
                            NumberAnimation { property: "scale"; from: 1.0; to: 0.95; duration: 100 }
                        }
                    }

                    delegate: ItemDelegate {
                        width: filterBox.width
                        height: 36
                        highlighted: filterBox.highlightedIndex === index

                        background: Rectangle {
                            color: highlighted ? LingmoUI.Theme.highlightColor : "transparent"
                            radius: LingmoUI.Theme.smallRadius
                        }

                        contentItem: Label {
                            text: modelData
                            leftPadding: LingmoUI.Units.smallSpacing
                            rightPadding: LingmoUI.Units.smallSpacing
                            color: highlighted ? LingmoUI.Theme.highlightedTextColor 
                                             : LingmoUI.Theme.textColor
                            font: filterBox.font
                            elide: Text.ElideRight
                            verticalAlignment: Text.AlignVCenter
                            horizontalAlignment: Text.AlignLeft
                        }

                        // 添加鼠标悬停效果
                        HoverHandler {
                            id: hoverHandler
                        }

                        Rectangle {
                            anchors.fill: parent
                            color: LingmoUI.Theme.textColor
                            opacity: hoverHandler.hovered ? 0.1 : 0
                            radius: LingmoUI.Theme.smallRadius
                        }
                    }
                }

                Button {
                    text: qsTr("End Process")
                    Layout.preferredHeight: 36
                    enabled: processListView.currentIndex >= 0
                    onClicked: {
                        if (processListView.currentIndex >= 0) {
                            var process = processListView.model[processListView.currentIndex]
                            systemInfo.killProcess(process.pid)
                            processes.selectedPid = ""
                            processListView.currentIndex = -1
                        }
                    }
                }

                Button {
                    text: qsTr("Refresh")
                    Layout.preferredHeight: 36
                    onClicked: systemInfo.refreshProcesses()
                }
            }
        }

        // 进程列表
        Components.CardItem {
            Layout.fillWidth: true
            Layout.fillHeight: true

            ListView {
                id: processListView
                anchors.fill: parent
                anchors.margins: 1
                clip: true
                model: systemInfo.processList
                
                // 添加这个属性来禁止自动选择
                currentIndex: -1
                
                // 添加虚拟化
                cacheBuffer: 0
                displayMarginBeginning: 100
                displayMarginEnd: 100
                
                // 添加这个属性来保持选择状态
                highlightFollowsCurrentItem: false
                
                // 添加动画
                add: Transition {
                    NumberAnimation { property: "opacity"; from: 0; to: 1.0; duration: 200 }
                }
                remove: Transition {
                    NumberAnimation { property: "opacity"; from: 1.0; to: 0; duration: 200 }
                }
                displaced: Transition {
                    NumberAnimation { properties: "x,y"; duration: 200; easing.type: Easing.OutQuad }
                }

                // 表头
                header: Rectangle {
                    width: processListView.width
                    height: 36
                    color: LingmoUI.Theme.darkMode ? Qt.rgba(255, 255, 255, 0.1)
                                                  : Qt.rgba(0, 0, 0, 0.05)

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: LingmoUI.Units.largeSpacing
                        anchors.rightMargin: LingmoUI.Units.largeSpacing
                        spacing: LingmoUI.Units.largeSpacing

                        Components.HeaderLabel {
                            text: qsTr("Name")
                            Layout.fillWidth: true
                            Layout.minimumWidth: 200
                            onClicked: {
                                if (sortBy === "name")
                                    sortAscending = !sortAscending
                                else {
                                    sortBy = "name"
                                    sortAscending = false
                                }
                                updateProcessList()
                            }
                            showSortIndicator: sortBy === "name"
                            ascending: sortAscending
                        }

                        Components.HeaderLabel {
                            text: qsTr("CPU")
                            Layout.preferredWidth: 80
                            textAlignment: Text.AlignRight
                            onClicked: {
                                if (sortBy === "cpu")
                                    sortAscending = !sortAscending
                                else {
                                    sortBy = "cpu"
                                    sortAscending = false
                                }
                                updateProcessList()
                            }
                            showSortIndicator: sortBy === "cpu"
                            ascending: sortAscending
                        }

                        Components.HeaderLabel {
                            text: qsTr("Memory")
                            Layout.preferredWidth: 80
                            textAlignment: Text.AlignRight
                            onClicked: {
                                if (sortBy === "memory")
                                    sortAscending = !sortAscending
                                else {
                                    sortBy = "memory"
                                    sortAscending = false
                                }
                                updateProcessList()
                            }
                            showSortIndicator: sortBy === "memory"
                            ascending: sortAscending
                        }

                        Components.HeaderLabel {
                            text: qsTr("PID")
                            Layout.preferredWidth: 80
                            textAlignment: Text.AlignRight
                            onClicked: {
                                if (sortBy === "pid")
                                    sortAscending = !sortAscending
                                else {
                                    sortBy = "pid"
                                    sortAscending = false
                                }
                                updateProcessList()
                            }
                            showSortIndicator: sortBy === "pid"
                            ascending: sortAscending
                        }

                        Components.HeaderLabel {
                            text: qsTr("User")
                            Layout.preferredWidth: 100
                            onClicked: {
                                if (sortBy === "user")
                                    sortAscending = !sortAscending
                                else {
                                    sortBy = "user"
                                    sortAscending = false
                                }
                                updateProcessList()
                            }
                            showSortIndicator: sortBy === "user"
                            ascending: sortAscending
                        }
                    }
                }

                // 进程项委托
                delegate: ItemDelegate {
                    width: processListView.width
                    height: 36
                    highlighted: ListView.isCurrentItem

                    Component.onCompleted: {
                        if (modelData.pid === processes.selectedPid) {
                            processListView.currentIndex = index
                        }
                    }

                    background: Rectangle {
                        color: parent.highlighted ? LingmoUI.Theme.highlightColor
                                                : "transparent"
                        radius: LingmoUI.Theme.smallRadius
                    }

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: LingmoUI.Units.largeSpacing
                        anchors.rightMargin: LingmoUI.Units.largeSpacing
                        spacing: LingmoUI.Units.largeSpacing

                        Label {
                            text: modelData.name
                            Layout.fillWidth: true
                            Layout.minimumWidth: 200
                            elide: Text.ElideRight
                            color: parent.parent.highlighted ? LingmoUI.Theme.highlightedTextColor
                                                           : LingmoUI.Theme.textColor
                        }

                        Label {
                            text: {
                                let cpu = Number(modelData.cpu)
                                return cpu > 0 ? modelData.cpu + "%" : "-"
                            }
                            Layout.preferredWidth: 80
                            horizontalAlignment: Text.AlignRight
                            opacity: text === "-" ? 0.5 : 1.0
                            color: parent.parent.highlighted ? LingmoUI.Theme.highlightedTextColor
                                                           : LingmoUI.Theme.textColor
                        }

                        Label {
                            text: modelData.memory
                            Layout.preferredWidth: 80
                            horizontalAlignment: Text.AlignRight
                            color: parent.parent.highlighted ? LingmoUI.Theme.highlightedTextColor
                                                           : LingmoUI.Theme.textColor
                        }

                        Label {
                            text: modelData.pid
                            Layout.preferredWidth: 80
                            horizontalAlignment: Text.AlignRight
                            color: parent.parent.highlighted ? LingmoUI.Theme.highlightedTextColor
                                                           : LingmoUI.Theme.textColor
                        }

                        Label {
                            text: modelData.user
                            Layout.preferredWidth: 100
                            color: parent.parent.highlighted ? LingmoUI.Theme.highlightedTextColor
                                                           : LingmoUI.Theme.textColor
                        }
                    }

                    MouseArea {
                        anchors.fill: parent
                        acceptedButtons: Qt.RightButton | Qt.LeftButton
                        onClicked: {
                            processListView.currentIndex = index
                            processes.selectedPid = modelData.pid
                            if (mouse.button === Qt.RightButton) {
                                processes.activeContextMenu = contextMenu
                                contextMenu.popup()
                            }
                        }
                    }

                    Menu {
                        id: contextMenu

                        MenuItem {
                            text: qsTr("End Process")
                            onTriggered: systemInfo.killProcess(modelData.pid)
                        }

                        Menu {
                            id: priorityMenu
                            title: qsTr("Priority")

                            // 添加自定义箭头
                            delegate: MenuItem {
                                id: menuItem
                                
                                contentItem: RowLayout {
                                    spacing: LingmoUI.Units.smallSpacing

                                    Label {
                                        text: menuItem.text
                                        color: menuItem.enabled ? LingmoUI.Theme.textColor 
                                                              : LingmoUI.Theme.disabledTextColor
                                        Layout.fillWidth: true
                                    }

                                    Image {
                                        width: 12
                                        height: 12
                                        source: "qrc:/images/go-down.svg"
                                        rotation: 270  // 旋转箭头指向右侧
                                        sourceSize.width: width
                                        sourceSize.height: height
                                        opacity: menuItem.enabled ? 1.0 : 0.3
                                        Layout.alignment: Qt.AlignRight
                                        visible: menuItem.subMenu
                                    }
                                }
                            }

                            MenuItem {
                                text: qsTr("Very High")
                                onTriggered: systemInfo.updateProcessPriority(modelData.pid, -20)
                            }
                            MenuItem {
                                text: qsTr("High")
                                onTriggered: systemInfo.updateProcessPriority(modelData.pid, -10)
                            }
                            MenuItem {
                                text: qsTr("Normal")
                                onTriggered: systemInfo.updateProcessPriority(modelData.pid, 0)
                            }
                            MenuItem {
                                text: qsTr("Low")
                                onTriggered: systemInfo.updateProcessPriority(modelData.pid, 10)
                            }
                            MenuItem {
                                text: qsTr("Very Low")
                                onTriggered: systemInfo.updateProcessPriority(modelData.pid, 19)
                            }
                        }

                        MenuSeparator { }

                        MenuItem {
                            text: qsTr("Details")
                            enabled: false
                        }
                    }
                }

                ScrollBar.vertical: ScrollBar {}
            }

            BusyIndicator {
                anchors.centerIn: parent
                running: processListView.count === 0
                visible: running
            }

            Label {
                anchors.centerIn: parent
                text: qsTr("No processes found")
                visible: processListView.count === 0 && !searchField.text
                opacity: 0.5
            }

            Label {
                anchors.centerIn: parent
                text: qsTr("No matching processes found")
                visible: processListView.count === 0 && searchField.text
                opacity: 0.5
            }
        }
    }

    // 更新进程列表
    function updateProcessList() {
        var userFilter = ""
        if (filterBox.currentIndex === 1)
            userFilter = systemInfo.currentUser()
        
        var activeOnly = filterBox.currentIndex === 2
        
        var scrollPos = processListView.contentY
        var currentPid = processListView.currentIndex >= 0 ? 
            processListView.model[processListView.currentIndex].pid : ""
        
        // 如果菜单正在显示，暂时不更新
        if (processes.activeContextMenu && processes.activeContextMenu.visible) {
            return
        }

        processListView.model = systemInfo.filteredProcessList(
            searchField.text,
            sortBy,
            sortAscending
        )

        // 如果之前有选中的进程，找到它的新位置
        if (currentPid) {
            for (var i = 0; i < processListView.model.length; i++) {
                if (processListView.model[i].pid === currentPid) {
                    processListView.currentIndex = i
                    break
                }
            }
        }

        processListView.contentY = scrollPos
    }

    Timer {
        id: searchTimer
        interval: 300
        onTriggered: updateProcessList()
    }

    Timer {
        interval: 2000
        running: true
        repeat: true
        onTriggered: updateProcessList()
    }
} 