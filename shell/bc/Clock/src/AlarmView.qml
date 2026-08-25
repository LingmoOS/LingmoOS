import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtGraphicalEffects 1.0
import LingmoUI 1.0 as LingmoUI
import QtMultimedia 5.15

ItemPage {
    // 移动 Component 到 Item 内部
    id: alarmView
    headerTitle: qsTr("Alarm")
    Component {
        id: checkBoxStyle
        Rectangle {
            implicitWidth: 20
            implicitHeight: 20
            radius: 4
            color: "transparent"
            border.width: 1
            border.color: LingmoUI.Theme.darkMode ? 
                Qt.rgba(1, 1, 1, 0.15) : Qt.rgba(0, 0, 0, 0.15)

            // 选中状态的背景
            Rectangle {
                anchors.fill: parent
                radius: 4
                color: LingmoUI.Theme.highlightColor
                visible: parent.parent.checked
                opacity: parent.parent.checked ? 1 : 0

                Behavior on opacity {
                    NumberAnimation { duration: 100 }
                }
            }

            // 对号图标
            Text {
                anchors.centerIn: parent
                text: "✓"
                font.pixelSize: 16  // 增大对号大小
                font.bold: true     // 加粗
                color: "white"
                visible: parent.parent.checked
                opacity: parent.parent.checked ? 1 : 0
                scale: parent.parent.checked ? 1 : 0.5  // 添加缩放动画

                Behavior on opacity {
                    NumberAnimation { duration: 100 }
                }
                Behavior on scale {
                    NumberAnimation { duration: 100 }
                }
            }
        }
    }

    property var alarmModel: ListModel {
        id: alarmListModel
        dynamicRoles: true
    }

    property color backgroundColor: LingmoUI.Theme.darkMode ? "#363636" : "#FFFFFF"
    property color hoveredColor: LingmoUI.Theme.darkMode ? Qt.lighter(backgroundColor, 1.3)
                                                       : Qt.darker(backgroundColor, 1.1)
    property color pressedColor: LingmoUI.Theme.darkMode ? Qt.lighter(backgroundColor, 1.1)
                                                       : Qt.darker(backgroundColor, 1.2)

    Loader {
        id: notificationDialogLoader
        sourceComponent: notificationDialogComponent
        active: false

        onLoaded: {
            item.notificationTitle = qsTr("闹钟")
            item.notificationText = alarmLabel || qsTr("闹钟时间到")
            item.open()
        }
    }

    function showNotification(label) {
        alarmLabel = label
        notificationDialogLoader.active = true
    }

    Timer {
        interval: 1000
        running: true
        repeat: true
        onTriggered: {
            var currentDate = new Date()
            var currentTime = currentDate.getHours().toString().padStart(2, '0') + ":" +
                             currentDate.getMinutes().toString().padStart(2, '0')
            var currentDay = currentDate.getDay()
            
            // 检查所有闹钟
            for (var i = 0; i < alarmModel.count; i++) {
                var alarm = alarmModel.get(i)
                if (alarm.enabled && alarm.time === currentTime) {
                    var shouldRing = false
                    
                    // 检查是否需要在当前日期响铃
                    if (alarm.repeat.sunday && currentDay === 0) shouldRing = true
                    if (alarm.repeat.monday && currentDay === 1) shouldRing = true
                    if (alarm.repeat.tuesday && currentDay === 2) shouldRing = true
                    if (alarm.repeat.wednesday && currentDay === 3) shouldRing = true
                    if (alarm.repeat.thursday && currentDay === 4) shouldRing = true
                    if (alarm.repeat.friday && currentDay === 5) shouldRing = true
                    if (alarm.repeat.saturday && currentDay === 6) shouldRing = true
                    
                    // 如果是一次性闹钟，或者当前日期匹配重复设置
                    if (!alarm.repeat.monday && !alarm.repeat.tuesday && 
                        !alarm.repeat.wednesday && !alarm.repeat.thursday && 
                        !alarm.repeat.friday && !alarm.repeat.saturday && 
                        !alarm.repeat.sunday) {
                        shouldRing = true
                        // 响铃后禁用一次性闹钟
                        alarmModel.setProperty(i, "enabled", false)
                    }
                    
                    if (shouldRing) {
                        showNotification(alarm.label)
                    }
                }
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: LingmoUI.Units.largeSpacing

        // 顶部工具栏
        RowLayout {
            Layout.fillWidth: true
            spacing: LingmoUI.Units.largeSpacing

            // Label {
            //     text: qsTr("闹钟")
            //     font.pixelSize: 24
            //     font.bold: true
            // }

            Item {
                Layout.fillWidth: true
            }

            Button {
                id: addAlarmButton
                text: qsTr("添加闹钟")
                icon.name: "list-add"
                background: Rectangle {
                    radius: LingmoUI.Theme.mediumRadius
                    color: addAlarmButton.pressed ? addAlarmButton.pressedColor : addAlarmButton.hovered ? addAlarmButton.hoveredColor : addAlarmButton.backgroundColor
                }
                scale: 1
                onClicked: addAlarmDialog.open()
            }
        }

        // 闹钟列表
        ListView {
            id: alarmListView
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: alarmModel
            spacing: LingmoUI.Units.smallSpacing
            clip: true

            delegate: Rectangle {
                width: ListView.view.width
                height: 80
                radius: LingmoUI.Theme.mediumRadius
                color: LingmoUI.Theme.backgroundColor

                // 添加阴影效果
                layer.enabled: true
                layer.effect: DropShadow {
                    transparentBorder: true
                    radius: 8
                    samples: 16
                    horizontalOffset: 0
                    verticalOffset: 2
                    color: Qt.rgba(0, 0, 0, 0.1)
                }

                // 添加鼠标悬停效果
                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
                    onEntered: parent.color = Qt.darker(LingmoUI.Theme.backgroundColor, 1.05)
                    onExited: parent.color = LingmoUI.Theme.backgroundColor
                    z: -1  // 放到最底层
                }

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: LingmoUI.Units.largeSpacing
                    spacing: LingmoUI.Units.largeSpacing

                    ColumnLayout {
                        spacing: LingmoUI.Units.smallSpacing

                        Label {
                            text: time
                            font.pixelSize: 24
                            color: LingmoUI.Theme.textColor
                        }

                        Label {
                            text: {
                                var days = []
                                if (repeat.monday) days.push(qsTr("周一"))
                                if (repeat.tuesday) days.push(qsTr("周二"))
                                if (repeat.wednesday) days.push(qsTr("周三"))
                                if (repeat.thursday) days.push(qsTr("周四"))
                                if (repeat.friday) days.push(qsTr("周五"))
                                if (repeat.saturday) days.push(qsTr("周六"))
                                if (repeat.sunday) days.push(qsTr("周日"))
                                return days.length > 0 ? days.join(" ") : qsTr("仅一次")
                            }
                            font.pixelSize: 13
                            opacity: 0.7
                            color: LingmoUI.Theme.textColor
                        }

                        Label {
                            text: label
                            font.pixelSize: 13
                            opacity: 0.7
                            visible: label !== ""
                            color: LingmoUI.Theme.textColor
                        }
                    }

                    Item {
                        Layout.fillWidth: true
                    }

                    Switch {
                        checked: model.enabled
                        onCheckedChanged: {
                            alarmModel.setProperty(index, "enabled", checked)
                        }
                    }

                    Button {
                        icon.name: "delete"
                        flat: true
                        onClicked: alarmModel.remove(index)
                    }
                }
            }

            // 空状态提示
            Item {
                visible: alarmListView.count === 0
                anchors.centerIn: parent
                width: 200
                height: column.height

                ColumnLayout {
                    id: column
                    anchors.centerIn: parent
                    spacing: LingmoUI.Units.largeSpacing

                    Image {
                        Layout.alignment: Qt.AlignHCenter
                        source: "image://icontheme/alarm"
                        sourceSize.width: 64
                        sourceSize.height: 64
                        opacity: 0.5
                    }

                    Label {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("没有闹钟")
                        font.pixelSize: 16
                        opacity: 0.7
                    }

                    Label {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("点击添加按钮创建一个新闹钟")
                        font.pixelSize: 13
                        opacity: 0.5
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignHCenter
                    }
                }
            }
        }
    }

    // 修改背景遮罩
    Rectangle {
        id: dimBackground
        anchors.fill: parent
        color: LingmoUI.Theme.darkMode ? 
            Qt.rgba(0, 0, 0, 0.8) : Qt.rgba(0, 0, 0, 0.6)  // 增加不透明度
        opacity: addAlarmDialog.opened ? 1.0 : 0
        visible: opacity > 0

        // 添加模糊效果
        layer.enabled: true
        layer.effect: FastBlur {
            radius: 32
        }
        
        // 添加渐变动画
        Behavior on opacity {
            NumberAnimation { 
                duration: 200
                easing.type: Easing.OutCubic
            }
        }

        // 点击背景关闭对话框
        MouseArea {
            anchors.fill: parent
            onClicked: addAlarmDialog.close()
            enabled: addAlarmDialog.opened
        }
    }

    // 修改对话框背景，增加阴影效果
    Dialog {
        id: addAlarmDialog
        title: qsTr("添加闹钟")
        parent: Overlay.overlay
        modal: true
        
        // 基本属性
        width: 320
        height: Math.min(parent.height - 100, 500)

        // 居中显示
        x: Math.round((parent.width - width) / 2)
        y: Math.round((parent.height - height) / 2)

        // 背景样式
        background: Rectangle {
            radius: LingmoUI.Theme.mediumRadius
            color: LingmoUI.Theme.backgroundColor
            border.width: 1
            border.color: Qt.rgba(0, 0, 0, 0.1)

            // 添加阴影
            layer.enabled: true
            layer.effect: DropShadow {
                radius: 20
                samples: 25
                color: Qt.rgba(0, 0, 0, 0.2)
                horizontalOffset: 0
                verticalOffset: 0
            }
        }

        // 动画效果
        enter: Transition {
            NumberAnimation { 
                property: "opacity"
                from: 0
                to: 1
                duration: 200
                easing.type: Easing.OutCubic
            }
            NumberAnimation { 
                property: "scale"
                from: 0.9
                to: 1
                duration: 200
                easing.type: Easing.OutCubic
            }
        }

        exit: Transition {
            NumberAnimation { 
                property: "opacity"
                from: 1
                to: 0
                duration: 200
                easing.type: Easing.InCubic
            }
            NumberAnimation { 
                property: "scale"
                from: 1
                to: 0.9
                duration: 200
                easing.type: Easing.InCubic
            }
        }

        // 对话框内容
        ColumnLayout {
            anchors.fill: parent
            anchors.margins: LingmoUI.Units.largeSpacing
            spacing: LingmoUI.Units.largeSpacing

            // 时间选择器
            ColumnLayout {
                Layout.alignment: Qt.AlignHCenter
                spacing: LingmoUI.Units.largeSpacing

                // 时间显示和选择
                RowLayout {
                    Layout.alignment: Qt.AlignHCenter
                    spacing: LingmoUI.Units.largeSpacing * 2

                    // 小时选择
                    ColumnLayout {
                        spacing: LingmoUI.Units.smallSpacing

                        Button {
                            id: hoursUpButton
                            Layout.preferredWidth: 100
                            flat: true
                            icon.name: "go-up"
                            icon.color: LingmoUI.Theme.textColor
                            opacity: hovered ? 0.8 : 0.5
                            onClicked: incrementHours()

                            background: Rectangle {
                                radius: LingmoUI.Theme.mediumRadius
                                color: hoursUpButton.pressed ? hoursUpButton.pressedColor 
                                                           : hoursUpButton.hovered ? hoursUpButton.hoveredColor 
                                                           : hoursUpButton.backgroundColor
                                
                                Behavior on color {
                                    ColorAnimation { duration: 100 }
                                }
                            }

                            scale: pressed ? 0.95 : 1.0
                            
                            Behavior on scale {
                                NumberAnimation { 
                                    duration: 100
                                    easing.type: Easing.OutCubic
                                }
                            }
                        }

                        TextField {
                            id: hoursField
                            Layout.preferredWidth: 100
                            text: "00"
                            horizontalAlignment: Text.AlignHCenter
                            font {
                                pixelSize: 32
                                family: "Monospace"
                                bold: true
                            }
                            color: LingmoUI.Theme.textColor
                            validator: IntValidator { bottom: 0; top: 23 }
                            selectByMouse: true
                            
                            background: Rectangle {
                                color: LingmoUI.Theme.darkMode ? "#363636" : "#FFFFFF"
                                radius: LingmoUI.Theme.smallRadius
                                border.width: 1
                                border.color: Qt.rgba(0, 0, 0, 0.1)
                            }

                            // 失去焦点时格式化
                            onEditingFinished: {
                                var value = parseInt(text) || 0
                                if (value > 23) value = 23
                                text = value.toString().padStart(2, '0')
                            }

                            // 支持上下键调整
                            Keys.onUpPressed: incrementHours()
                            Keys.onDownPressed: decrementHours()

                            // 添加数值变化动画
                            property real displayValue: parseFloat(text)
                            Behavior on displayValue {
                                NumberAnimation { 
                                    duration: 200
                                    easing.type: Easing.OutCubic
                                }
                            }

                            // 添加文本变化时的缩放动画
                            scale: 1
                            Behavior on scale {
                                SequentialAnimation {
                                    NumberAnimation { to: 0.95; duration: 50 }
                                    NumberAnimation { to: 1.0; duration: 100 }
                                }
                            }

                            onTextChanged: {
                                scale = 1  // 触发缩放动画
                            }
                        }

                        Button {
                            id: hoursDownButton
                            Layout.preferredWidth: 100
                            flat: true
                            icon.name: "go-down"
                            icon.color: LingmoUI.Theme.textColor
                            opacity: hovered ? 0.8 : 0.5
                            onClicked: decrementHours()

                            background: Rectangle {
                                radius: LingmoUI.Theme.mediumRadius
                                color: hoursDownButton.pressed ? hoursDownButton.pressedColor 
                                                           : hoursDownButton.hovered ? hoursDownButton.hoveredColor 
                                                           : hoursDownButton.backgroundColor
                            }
                            scale: pressed ? 0.95 : 1.0
                            
                            Behavior on scale {
                                NumberAnimation { 
                                    duration: 100
                                    easing.type: Easing.OutCubic
                                }
                            }
                        }
                    }

                    Label {
                        text: ":"
                        font {
                            pixelSize: 32
                            bold: true
                        }
                        color: LingmoUI.Theme.textColor
                        opacity: 0.8
                        Layout.alignment: Qt.AlignVCenter
                    }

                    // 分钟选择
                    ColumnLayout {
                        spacing: LingmoUI.Units.smallSpacing

                        Button {
                            id: minutesUpButton
                            Layout.preferredWidth: 100
                            flat: true
                            icon.name: "go-up"
                            icon.color: LingmoUI.Theme.textColor
                            opacity: hovered ? 0.8 : 0.5
                            onClicked: incrementMinutes()

                            background: Rectangle {
                                radius: LingmoUI.Theme.mediumRadius
                                color: minutesUpButton.pressed ? minutesUpButton.pressedColor 
                                                           : minutesUpButton.hovered ? minutesUpButton.hoveredColor 
                                                           : minutesUpButton.backgroundColor
                            }
                            scale: pressed ? 0.95 : 1.0
                            
                            Behavior on scale {
                                NumberAnimation { 
                                    duration: 100
                                    easing.type: Easing.OutCubic
                                }
                            }
                        }

                        TextField {
                            id: minutesField
                            Layout.preferredWidth: 100
                            text: "00"
                            horizontalAlignment: Text.AlignHCenter
                            font {
                                pixelSize: 32
                                family: "Monospace"
                                bold: true
                            }
                            color: LingmoUI.Theme.textColor
                            validator: IntValidator { bottom: 0; top: 59 }
                            selectByMouse: true
                            
                            background: Rectangle {
                                color: LingmoUI.Theme.darkMode ? "#363636" : "#FFFFFF"
                                radius: LingmoUI.Theme.smallRadius
                                border.width: 1
                                border.color: Qt.rgba(0, 0, 0, 0.1)
                            }

                            // 失去焦点时格式化
                            onEditingFinished: {
                                var value = parseInt(text) || 0
                                if (value > 59) value = 59
                                text = value.toString().padStart(2, '0')
                            }

                            // 支持上下键调整
                            Keys.onUpPressed: incrementMinutes()
                            Keys.onDownPressed: decrementMinutes()
                        }

                        Button {
                            id: minutesDownButton
                            Layout.preferredWidth: 100
                            flat: true
                            icon.name: "go-down"
                            icon.color: LingmoUI.Theme.textColor
                            opacity: hovered ? 0.8 : 0.5
                            onClicked: decrementMinutes()

                            background: Rectangle {
                                radius: LingmoUI.Theme.mediumRadius
                                color: minutesDownButton.pressed ? minutesDownButton.pressedColor 
                                                           : minutesDownButton.hovered ? minutesDownButton.hoveredColor 
                                                           : minutesDownButton.backgroundColor
                            }
                            scale: pressed ? 0.95 : 1.0
                            
                            Behavior on scale {
                                NumberAnimation { 
                                    duration: 100
                                    easing.type: Easing.OutCubic
                                }
                            }
                        }
                    }
                }
            }

            // 重复选项
            GroupBox {
                title: qsTr("重复")
                Layout.fillWidth: true
                background: Rectangle {
                    color: "transparent"
                    border.width: 1
                    border.color: Qt.rgba(0, 0, 0, 0.1)
                    radius: LingmoUI.Theme.smallRadius
                }

                GridLayout {
                    columns: 4
                    rowSpacing: LingmoUI.Units.smallSpacing
                    columnSpacing: LingmoUI.Units.smallSpacing

                    CheckBox {
                        id: mondayCheckBox
                        text: qsTr("周一")
                        indicator: checkBoxStyle.createObject(mondayCheckBox)
                    }
                    CheckBox {
                        id: tuesdayCheckBox
                        text: qsTr("周二")
                        indicator: checkBoxStyle.createObject(tuesdayCheckBox)
                    }
                    CheckBox {
                        id: wednesdayCheckBox
                        text: qsTr("周三")
                        indicator: checkBoxStyle.createObject(wednesdayCheckBox)
                    }
                    CheckBox {
                        id: thursdayCheckBox
                        text: qsTr("周四")
                        indicator: checkBoxStyle.createObject(thursdayCheckBox)
                    }
                    CheckBox {
                        id: fridayCheckBox
                        text: qsTr("周五")
                        indicator: checkBoxStyle.createObject(fridayCheckBox)
                    }
                    CheckBox {
                        id: saturdayCheckBox
                        text: qsTr("周六")
                        indicator: checkBoxStyle.createObject(saturdayCheckBox)
                    }
                    CheckBox {
                        id: sundayCheckBox
                        text: qsTr("周日")
                        indicator: checkBoxStyle.createObject(sundayCheckBox)
                    }
                }
            }

            // 标签输入
            TextField {
                id: labelField
                Layout.fillWidth: true
                placeholderText: qsTr("闹钟标签（可选）")
                background: Rectangle {
                    color: LingmoUI.Theme.darkMode ? "#363636" : "#FFFFFF"
                    radius: LingmoUI.Theme.smallRadius
                    border.width: 1
                    border.color: Qt.rgba(0, 0, 0, 0.1)
                }
            }

            Item {
                Layout.fillHeight: true
            }

            // 按钮
            DialogButtonBox {
                Layout.fillWidth: true
                standardButtons: DialogButtonBox.Ok | DialogButtonBox.Cancel
                alignment: Qt.AlignRight

                background: Rectangle {
                    color: "transparent"
                }

                delegate: Button {
                    id: dialogButton
                    property bool isOkButton: DialogButtonBox.buttonRole === DialogButtonBox.AcceptRole
                    property color backgroundColor: isOkButton ? 
                        LingmoUI.Theme.highlightColor : 
                        (LingmoUI.Theme.darkMode ? "#363636" : "#FFFFFF")
                    property color hoveredColor: isOkButton ? 
                        Qt.lighter(LingmoUI.Theme.highlightColor, 1.1) :
                        (LingmoUI.Theme.darkMode ? Qt.lighter("#363636", 1.3) : Qt.darker("#FFFFFF", 1.1))
                    property color pressedColor: isOkButton ? 
                        Qt.darker(LingmoUI.Theme.highlightColor, 1.1) :
                        (LingmoUI.Theme.darkMode ? Qt.lighter("#363636", 1.1) : Qt.darker("#FFFFFF", 1.2))

                    // 统一按钮宽度
                    implicitWidth: 80
                    
                    contentItem: Label {
                        text: dialogButton.text
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        color: isOkButton ? "white" : LingmoUI.Theme.textColor
                    }
                    
                    background: Rectangle {
                        radius: LingmoUI.Theme.mediumRadius
                        color: dialogButton.pressed ? dialogButton.pressedColor 
                                                  : dialogButton.hovered ? dialogButton.hoveredColor 
                                                  : dialogButton.backgroundColor
                    }

                    // 设置确定按钮的突出显示
                    highlighted: DialogButtonBox.buttonRole === DialogButtonBox.AcceptRole
                }

                onAccepted: {
                    alarmModel.append({
                        "time": hoursField.text + ":" + minutesField.text,
                        "enabled": true,  // 确保这个属性存在
                        "label": labelField.text,
                        "repeat": {
                            "monday": mondayCheckBox.checked,
                            "tuesday": tuesdayCheckBox.checked,
                            "wednesday": wednesdayCheckBox.checked,
                            "thursday": thursdayCheckBox.checked,
                            "friday": fridayCheckBox.checked,
                            "saturday": saturdayCheckBox.checked,
                            "sunday": sundayCheckBox.checked
                        }
                    })
                    addAlarmDialog.close()
                }
                onRejected: addAlarmDialog.close()
            }
        }

        // 关闭时重置表单
        onClosed: {
            hoursField.text = "00"
            minutesField.text = "00"
            mondayCheckBox.checked = false
            tuesdayCheckBox.checked = false
            wednesdayCheckBox.checked = false
            thursdayCheckBox.checked = false
            fridayCheckBox.checked = false
            saturdayCheckBox.checked = false
            sundayCheckBox.checked = false
            labelField.text = ""
        }
    }

    // 添加辅助函数
    function incrementHours() {
        var newValue = (parseInt(hoursField.text) + 1) % 24
        hoursField.text = newValue.toString().padStart(2, '0')
    }

    function decrementHours() {
        var newValue = (parseInt(hoursField.text) - 1 + 24) % 24
        hoursField.text = newValue.toString().padStart(2, '0')
    }

    function incrementMinutes() {
        var newValue = (parseInt(minutesField.text) + 1) % 60
        minutesField.text = newValue.toString().padStart(2, '0')
    }

    function decrementMinutes() {
        var newValue = (parseInt(minutesField.text) - 1 + 60) % 60
        minutesField.text = newValue.toString().padStart(2, '0')
    }
} 