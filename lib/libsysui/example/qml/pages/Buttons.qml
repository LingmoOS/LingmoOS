import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import LingmoUI

LingmoScrollablePage {
    title: qsTr("Buttons")

    LingmoText {
        text: qsTr("Here are some examples for various buttons!")
    }

    LingmoFrame {
        Layout.fillWidth: true
        Layout.preferredHeight: 68
        Layout.topMargin: 10

        leftPadding: 10

        LingmoTextButton {
            
            text: qsTr("Text Button")
            onClicked: {
                showInfo("点击Text Button")
            }
            anchors{
                verticalCenter: parent.verticalCenter
                left: parent.left
            }
        }
    }

    LingmoFrame {
        Layout.fillWidth: true
        Layout.preferredHeight: 68
        Layout.topMargin: 10

        leftPadding: 10

        LingmoButton {
            text: qsTr("Button")
            onClicked: {
                showInfo("点击Button")
            }
            anchors{
                verticalCenter: parent.verticalCenter
                left: parent.left
            }
        }
    }

    LingmoFrame {
        Layout.fillWidth: true
        Layout.preferredHeight: 68
        Layout.topMargin: 10

        leftPadding: 10

        LingmoProgressButton {
            id: btn_progress
            text: qsTr("Progress Button")
            onClicked: {
                btn_progress.progress = 0
                timer_progress.restart()
            }
            anchors{
                verticalCenter: parent.verticalCenter
                left: parent.left
            }
        }

        Timer{
            id: timer_progress
            interval: 200
            onTriggered: {
                btn_progress.progress = (btn_progress.progress + 0.1).toFixed(1)
                if(btn_progress.progress === 1){
                    timer_progress.stop()
                }else{
                    timer_progress.start()
                }
            }
        }
    }

    LingmoFrame {
        Layout.fillWidth: true
        Layout.preferredHeight: 68
        Layout.topMargin: 10

        leftPadding: 10

        LingmoDelayButton {
            text: qsTr("Delay Button")
            delay: 1000
            onActivated: {
                showInfo("激活了 Delay Button!")
            }
            anchors{
                verticalCenter: parent.verticalCenter
                left: parent.left
            }
        }
    }

    LingmoFrame {
        Layout.fillWidth: true
        Layout.preferredHeight: 68
        Layout.topMargin: 10

        leftPadding: 10

        LingmoLoadingButton {
            id: btn_loading
            loading: true
            text: qsTr("Loading Button")
            disabled: false
            onClicked: {
                btn_loading.loading = false
                showInfo("点击了 Loading Button!")
            }
            anchors{
                verticalCenter: parent.verticalCenter
                left: parent.left
            }
        }
    }
}
