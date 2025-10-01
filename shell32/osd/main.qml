import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import App 1.0   // VolumeController C++ 类型

ApplicationWindow {
    id: osdWindow
    width: 300
    height: 70
    visible: true
    flags: Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint | Qt.Tool
    color: "transparent"
    property int margin: 16
    property real scaleFactor: 1.0

    VolumeController { id: controller }

    Rectangle {
        id: osd
        anchors.fill: parent
        radius: 14 * scaleFactor
        color: "#AA222222"
        opacity: 0

        RowLayout {
            anchors.fill: parent
            anchors.margins: 12 * scaleFactor
            spacing: 12 * scaleFactor

            Image {
                id: volIcon
                width: 32 * scaleFactor
                height: 32 * scaleFactor
                scale: 1.0
                Behavior on scale {
                    SpringAnimation { spring: 0.2; damping: 0.25 }
                }
            }

            Rectangle {
                id: volBarBg
                Layout.preferredWidth: 200 * scaleFactor
                Layout.preferredHeight: 16 * scaleFactor
                radius: 8 * scaleFactor
                color: "#444444"

                Rectangle {
                    id: volBar
                    height: parent.height
                    radius: 8 * scaleFactor
                    width: 0
                    gradient: Gradient {
                        GradientStop { position: 0; color: "#33FF77" }
                        GradientStop { position: 1; color: "#11AA44" }
                    }
                    Behavior on width { SpringAnimation { spring: 0.15; damping: 0.25 } }
                }
            }

            Text {
                id: volText
                font.pixelSize: 20 * scaleFactor
                font.bold: true
                color: "white"
                Layout.alignment: Qt.AlignVCenter
            }
        }

        Timer {
            id: fadeTimer
            interval: 1000
            running: false
            repeat: false
            onTriggered: { osd.opacity = 0; osdWindow.visible = false }
        }

        Behavior on opacity { NumberAnimation { duration: 300; easing.type: Easing.InOutQuad } }
    }

    Timer {
        id: updateTimer
        interval: 200
        running: true
        repeat: true
        onTriggered: {
            if (VolumeController.volume > 0 || VolumeController.muted) {

                // 获取鼠标位置，通过 C++ 提供的 MouseHelper
                var mousePoint = MouseHelper.cursorPos()
                var mouseScreen = Qt.application.screenAt(mousePoint)
                if (!mouseScreen) mouseScreen = Qt.application.primaryScreen

                // 根据屏幕 DPI 自动缩放
                var scaleFactor = mouseScreen.logicalDotsPerInch / 96.0

                osdWindow.x = mouseScreen.geometry.x + osdWindow.margin * scaleFactor
                osdWindow.y = mouseScreen.geometry.y + osdWindow.margin * scaleFactor

                osdWindow.visible = true
                osd.opacity = 1

                // 更新图标
                volIcon.source = VolumeController.muted ? "qrc:/icons/audio-volume-muted-symbolic.svg"
                                : VolumeController.volume < 33 ? "qrc:/icons/audio-volume-low-symbolic.svg"
                                : VolumeController.volume < 66 ? "qrc:/icons/audio-volume-medium-symbolic.svg"
                                : "qrc:/icons/audio-volume-high-symbolic.svg"

                // 弹性动画更新宽度
                volBar.width = volBarBg.width * VolumeController.volume / 100

                // 图标轻微放大弹跳
                volIcon.scale = 1.2
                volText.text = VolumeController.muted ? "Muted" : VolumeController.volume + "%"

                // 恢复原始尺寸
                volIcon.scale = 1.0

                fadeTimer.start()
            }
        }
    }
}
