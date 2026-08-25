import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Qt.labs.platform 1.1
import QtGraphicalEffects 1.12
import LingmoUI 1.0 as LingmoUI

ItemPage {
    id: timerView
    headerTitle: qsTr("计时器")
    property int remainingTime: 0
    property int initialTime: 0

    NotificationDialog {
        id: timerNotification
        notificationTitle: qsTr("倒计时结束")
        notificationText: qsTr("设定的时间已到")
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: LingmoUI.Units.largeSpacing
        spacing: LingmoUI.Units.largeSpacing

        Item {
            Layout.fillHeight: true
        }

        // 时间显示
        Label {
            id: timerDisplay
            text: formatTime(remainingTime)
            font.pixelSize: 72
            font.family: "Monospace"
            Layout.alignment: Qt.AlignHCenter
            color: LingmoUI.Theme.textColor
        }

        // 时间设置
        Rectangle {
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredWidth: timeSettingLayout.implicitWidth + LingmoUI.Units.largeSpacing * 2
            Layout.preferredHeight: timeSettingLayout.implicitHeight + LingmoUI.Units.largeSpacing * 2
            color: LingmoUI.Theme.backgroundColor
            radius: LingmoUI.Theme.mediumRadius

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

            // 添加内部阴影
            Rectangle {
                id: innerShadow
                anchors.fill: parent
                radius: parent.radius
                color: "transparent"

                // 顶部渐变
                Rectangle {
                    anchors.top: parent.top
                    width: parent.width
                    height: parent.radius
                    gradient: Gradient {
                        GradientStop { position: 0.0; color: Qt.rgba(0, 0, 0, 0.03) }
                        GradientStop { position: 1.0; color: "transparent" }
                    }
                }

                // 底部渐变
                Rectangle {
                    anchors.bottom: parent.bottom
                    width: parent.width
                    height: parent.radius
                    gradient: Gradient {
                        GradientStop { position: 0.0; color: "transparent" }
                        GradientStop { position: 1.0; color: Qt.rgba(1, 1, 1, 0.03) }
                    }
                }
            }

            RowLayout {
                id: timeSettingLayout
                anchors.centerIn: parent
                spacing: LingmoUI.Units.largeSpacing

                Dial {
                    id: hoursSpinBox
                    from: 0
                    to: 23
                    value: 0
                    stepSize: 1
                    palette.dark: LingmoUI.Theme.highlightColor
                    Layout.preferredWidth: 100
                    Layout.preferredHeight: 100

                    Label {
                        anchors.centerIn: parent
                        text: Math.round(parent.value).toString().padStart(2, '0')
                        font.pixelSize: 20
                        font.family: "Monospace"
                    }

                    Label {
                        anchors.bottom: parent.bottom
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: qsTr("小时")
                        font.pixelSize: 12
                        opacity: 0.7
                    }
                }

                Dial {
                    id: minutesSpinBox
                    from: 0
                    to: 59
                    value: 0
                    stepSize: 1
                    palette.dark: LingmoUI.Theme.highlightColor
                    Layout.preferredWidth: 100
                    Layout.preferredHeight: 100

                    Label {
                        anchors.centerIn: parent
                        text: Math.round(parent.value).toString().padStart(2, '0')
                        font.pixelSize: 20
                        font.family: "Monospace"
                    }

                    Label {
                        anchors.bottom: parent.bottom
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: qsTr("分钟")
                        font.pixelSize: 12
                        opacity: 0.7
                    }
                }

                Dial {
                    id: secondsSpinBox
                    from: 0
                    to: 59
                    value: 0
                    stepSize: 1
                    palette.dark: LingmoUI.Theme.highlightColor
                    Layout.preferredWidth: 100
                    Layout.preferredHeight: 100

                    Label {
                        anchors.centerIn: parent
                        text: Math.round(parent.value).toString().padStart(2, '0')
                        font.pixelSize: 20
                        font.family: "Monospace"
                    }

                    Label {
                        anchors.bottom: parent.bottom
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: qsTr("秒钟")
                        font.pixelSize: 12
                        opacity: 0.7
                    }
                }
            }
        }

        // 控制按钮
        RowLayout {
            spacing: LingmoUI.Units.largeSpacing
            Layout.alignment: Qt.AlignHCenter
            Layout.topMargin: LingmoUI.Units.largeSpacing

            Button {
                text: qsTr("开始")
                flat: true
                icon.name: "media-playback-start"
                enabled: !timer.running && (hoursSpinBox.value > 0 || minutesSpinBox.value > 0 || secondsSpinBox.value > 0)
                onClicked: startTimer()
            }

            Button {
                text: qsTr("暂停")
                icon.name: "media-playback-pause"
                enabled: timer.running
                onClicked: timer.stop()
            }

            Button {
                text: qsTr("继续")
                icon.name: "media-playback-start"
                enabled: !timer.running && remainingTime > 0
                onClicked: timer.start()
            }

            Button {
                text: qsTr("重置")
                icon.name: "media-playlist-repeat"
                enabled: remainingTime > 0 || timer.running
                onClicked: resetTimer()
            }
        }

        Item {
            Layout.fillHeight: true
        }

        Timer {
            id: timer
            interval: 1000
            repeat: true
            onTriggered: {
                if (remainingTime > 0) {
                    remainingTime--
                    if (remainingTime === 0) {
                        running = false
                        notificationDialogLoader.active = true
                    }
                }
            }
        }
    }

    function formatTime(seconds) {
        var hours = Math.floor(seconds / 3600)
        var minutes = Math.floor((seconds % 3600) / 60)
        var secs = seconds % 60
        return hours.toString().padStart(2, '0') + ":" +
               minutes.toString().padStart(2, '0') + ":" +
               secs.toString().padStart(2, '0')
    }

    function startTimer() {
        if (!timer.running && remainingTime === 0) {
            initialTime = Math.round(hoursSpinBox.value) * 3600 + 
                         Math.round(minutesSpinBox.value) * 60 + 
                         Math.round(secondsSpinBox.value)
            remainingTime = initialTime
        }
        timer.start()
    }

    function resetTimer() {
        timer.stop()
        remainingTime = 0
        hoursSpinBox.value = 0
        minutesSpinBox.value = 0
        secondsSpinBox.value = 0
    }

    Loader {
        id: notificationDialogLoader
        sourceComponent: notificationDialogComponent
        active: false

        onLoaded: {
            item.notificationTitle = qsTr("倒计时结束")
            item.notificationText = qsTr("设定的时间已到")
            item.open()
        }
    }
} 