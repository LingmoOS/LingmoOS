import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtGraphicalEffects 1.0
import LingmoUI 1.0 as LingmoUI

ItemPage {
    id: worldClockView
    headerTitle: qsTr("World Clock")
    property var timezones: ({
        "北京": { offset: 8, name: "Asia/Shanghai" },
        "东京": { offset: 9, name: "Asia/Tokyo" },
        "首尔": { offset: 9, name: "Asia/Seoul" },
        "新加坡": { offset: 8, name: "Asia/Singapore" },
        "曼谷": { offset: 7, name: "Asia/Bangkok" },
        "新德里": { offset: 5.5, name: "Asia/Kolkata" },
        "迪拜": { offset: 4, name: "Asia/Dubai" },
        "莫斯科": { offset: 3, name: "Europe/Moscow" },
        "伊斯坦布尔": { offset: 3, name: "Europe/Istanbul" },
        "柏林": { offset: 1, name: "Europe/Berlin" },
        "巴黎": { offset: 1, name: "Europe/Paris" },
        "伦敦": { offset: 0, name: "Europe/London" },
        "开罗": { offset: 2, name: "Africa/Cairo" },
        "约翰内斯堡": { offset: 2, name: "Africa/Johannesburg" },
        "纽约": { offset: -5, name: "America/New_York" },
        "芝加哥": { offset: -6, name: "America/Chicago" },
        "丹佛": { offset: -7, name: "America/Denver" },
        "洛杉矶": { offset: -8, name: "America/Los_Angeles" },
        "温哥华": { offset: -8, name: "America/Vancouver" },
        "檀香山": { offset: -10, name: "Pacific/Honolulu" },
        "悉尼": { offset: 10, name: "Australia/Sydney" },
        "墨尔本": { offset: 10, name: "Australia/Melbourne" },
        "奥克兰": { offset: 12, name: "Pacific/Auckland" }
    })

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: LingmoUI.Units.largeSpacing
        spacing: LingmoUI.Units.largeSpacing

        // 添加城市按钮
        Button {
            text: qsTr("添加城市")
            icon.name: "list-add"
            Layout.alignment: Qt.AlignRight
            onClicked: addCityDialog.open()
        }

        // 城市列表
        ListView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: ListModel {
                id: cityModel
                Component.onCompleted: {
                    append({
                        "city": "北京",
                        "timezone": "UTC+8"
                    })
                }
            }
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
                    z: -1
                }

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: LingmoUI.Units.largeSpacing
                    spacing: LingmoUI.Units.largeSpacing

                    ColumnLayout {
                        spacing: LingmoUI.Units.smallSpacing

                        Label {
                            text: city
                            font.pixelSize: 24
                            color: LingmoUI.Theme.textColor
                        }

                        Label {
                            text: timezone
                            font.pixelSize: 13
                            opacity: 0.7
                            color: LingmoUI.Theme.textColor
                        }
                    }

                    Item {
                        Layout.fillWidth: true
                    }

                    Label {
                        property int offset: timezones[city].offset
                        text: {
                            var d = new Date()
                            var utc = d.getTime() + (d.getTimezoneOffset() * 60000)
                            var nd = new Date(utc + (3600000 * offset))
                            return nd.toLocaleTimeString(Qt.locale(), "HH:mm:ss")
                        }
                        font.pixelSize: 24
                        font.family: "Monospace"
                        color: LingmoUI.Theme.textColor
                    }

                    Button {
                        icon.name: "delete"
                        flat: true
                        onClicked: cityModel.remove(index)
                    }
                }
            }
        }
    }

    // 添加背景遮罩
    Rectangle {
        id: dimBackground
        anchors.fill: parent
        color: LingmoUI.Theme.darkMode ? 
            Qt.rgba(0, 0, 0, 0.8) : Qt.rgba(0, 0, 0, 0.6)
        opacity: addCityDialog.opened ? 1.0 : 0
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
            onClicked: addCityDialog.close()
            enabled: addCityDialog.opened
        }
    }

    Dialog {
        id: addCityDialog
        title: qsTr("添加城市")
        parent: Overlay.overlay
        modal: true
        width: 320
        height: contentHeight + header.height + footer.height + LingmoUI.Units.largeSpacing * 2

        // 居中显示
        x: Math.round((parent.width - width) / 2)
        y: Math.round((parent.height - height) / 2)

        // 修改内容区域
        contentItem: ColumnLayout {
            spacing: LingmoUI.Units.largeSpacing

            ComboBox {
                id: cityComboBox
                Layout.fillWidth: true
                Layout.preferredHeight: 36
                model: [
                    "北京 (UTC+8)",
                    "东京 (UTC+9)",
                    "首尔 (UTC+9)",
                    "新加坡 (UTC+8)",
                    "曼谷 (UTC+7)",
                    "新德里 (UTC+5:30)",
                    "迪拜 (UTC+4)",
                    "莫斯科 (UTC+3)",
                    "伊斯坦布尔 (UTC+3)",
                    "柏林 (UTC+1)",
                    "巴黎 (UTC+1)",
                    "伦敦 (UTC+0)",
                    "开罗 (UTC+2)",
                    "约翰内斯堡 (UTC+2)",
                    "纽约 (UTC-5)",
                    "芝加哥 (UTC-6)",
                    "丹佛 (UTC-7)",
                    "洛杉矶 (UTC-8)",
                    "温哥华 (UTC-8)",
                    "檀香山 (UTC-10)",
                    "悉尼 (UTC+10)",
                    "墨尔本 (UTC+10)",
                    "奥克兰 (UTC+12)"
                ]

                background: Rectangle {
                    radius: LingmoUI.Theme.smallRadius
                    color: LingmoUI.Theme.darkMode ? "#363636" : "#FFFFFF"
                    border.width: 1
                    border.color: Qt.rgba(0, 0, 0, 0.1)
                }

                popup: Popup {
                    y: cityComboBox.height
                    width: cityComboBox.width
                    implicitHeight: Math.min(contentItem.implicitHeight + padding * 2, 300)
                    padding: 1

                    contentItem: ListView {
                        clip: true
                        implicitHeight: contentHeight
                        model: cityComboBox.popup.visible ? cityComboBox.delegateModel : null
                        currentIndex: cityComboBox.highlightedIndex
                        ScrollIndicator.vertical: ScrollIndicator { }
                    }

                    background: Rectangle {
                        color: LingmoUI.Theme.backgroundColor
                        radius: LingmoUI.Theme.smallRadius
                        border.width: 1
                        border.color: Qt.rgba(0, 0, 0, 0.1)
                    }
                }
            }

            // 按钮区域
            RowLayout {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignRight
                spacing: LingmoUI.Units.smallSpacing

                Button {
                    text: qsTr("取消")
                    onClicked: addCityDialog.reject()
                }

                Button {
                    text: qsTr("确定")
                    highlighted: true
                    onClicked: {
                        var city = cityComboBox.currentText.split(" ")[0]
                        if (!isCityExists(city)) {
                            cityModel.append({
                                "city": city,
                                "timezone": "UTC" + (timezones[city].offset >= 0 ? "+" : "") + 
                                    (timezones[city].offset % 1 === 0 ? 
                                        timezones[city].offset : 
                                        timezones[city].offset.toFixed(1))
                            })
                        }
                        addCityDialog.close()
                    }
                }
            }
        }

        // 背景样式
        background: Rectangle {
            radius: LingmoUI.Theme.mediumRadius
            color: LingmoUI.Theme.backgroundColor
            border.width: 1
            border.color: Qt.rgba(0, 0, 0, 0.1)

            layer.enabled: true
            layer.effect: DropShadow {
                radius: 20
                samples: 25
                color: Qt.rgba(0, 0, 0, 0.2)
                horizontalOffset: 0
                verticalOffset: 0
            }
        }
    }

    function isCityExists(cityName) {
        for (var i = 0; i < cityModel.count; i++) {
            if (cityModel.get(i).city === cityName) {
                return true
            }
        }
        return false
    }

    Timer {
        interval: 1000
        running: true
        repeat: true
        onTriggered: {
            // 不直接修改 count，而是使用 model.changed() 触发更新
            for (var i = 0; i < cityModel.count; i++) {
                var city = cityModel.get(i)
                var offset = timezones[city.city].offset
                var d = new Date()
                var utc = d.getTime() + (d.getTimezoneOffset() * 60000)
                var nd = new Date(utc + (3600000 * offset))
                city.currentTime = nd.toLocaleTimeString(Qt.locale(), "HH:mm:ss")
            }
        }
    }
} 