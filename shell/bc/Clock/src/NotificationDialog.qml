import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtGraphicalEffects 1.0
import LingmoUI 1.0 as LingmoUI
import Lingmo.Notification 1.0

Dialog {
    id: notificationDialog
    parent: Overlay.overlay
    modal: true
    width: 320
    height: contentHeight + header.height + footer.height + LingmoUI.Units.largeSpacing * 2
    dim: true  // 启用背景变暗

    // 居中显示
    x: Math.round((parent.width - width) / 2)
    y: Math.round((parent.height - height) / 2)

    // 自定义背景遮罩
    Overlay.modal: Rectangle {
        color: LingmoUI.Theme.darkMode ? 
            Qt.rgba(0, 0, 0, 0.4) : Qt.rgba(0, 0, 0, 0.2)

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
    }

    // 标题和内容
    property string notificationTitle: ""
    property string notificationText: ""

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

    // 标题
    header: Item {
        height: 50

        Label {
            anchors.left: parent.left
            anchors.leftMargin: LingmoUI.Units.largeSpacing
            anchors.verticalCenter: parent.verticalCenter
            text: notificationTitle
            font.bold: true
            font.pixelSize: 15
        }
    }

    // 内容区域
    contentItem: ColumnLayout {
        spacing: LingmoUI.Units.largeSpacing

        Label {
            text: notificationText
            Layout.fillWidth: true
            Layout.margins: LingmoUI.Units.largeSpacing
            wrapMode: Text.WordWrap
            font.pixelSize: 14
        }
    }

    // 按钮区域
    footer: DialogButtonBox {
        alignment: Qt.AlignRight
        background: Rectangle {
            color: "transparent"
        }

        Button {
            text: qsTr("确定")
            DialogButtonBox.buttonRole: DialogButtonBox.AcceptRole
            flat: true
            highlighted: true
        }

        onAccepted: {
            notificationDialog.close()
        }
    }

    // 修改入场动画
    enter: Transition {
        ParallelAnimation {
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
    }

    // 修改出场动画
    exit: Transition {
        ParallelAnimation {
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
    }

    Notification {
        id: notification
        title: notificationTitle
        body: notificationText
        hasSound: true
        silent: false
    }

    // 只在打开时发送一次通知
    onOpened: {
        notification.show()
    }

    Component.onDestruction: {
        // 清理资源
    }
} 