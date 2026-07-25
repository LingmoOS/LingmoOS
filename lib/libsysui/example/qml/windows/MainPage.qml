import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LingmoUI
import QtQuick.Controls.LingmoStyle as QQCL

LingmoWindow {
    id: window
    visible: true
    width: 640
    height: 480
    title: qsTr("LingmoUI Example")

    showDark: true

    property var colors: [LingmoColor.Yellow, LingmoColor.Orange, LingmoColor.Red, LingmoColor.Magenta, LingmoColor.Purple, LingmoColor.Blue, LingmoColor.Teal, LingmoColor.Green]

    function newTab() {
        tab_view.appendTab("qrc:/image/logo.svg", qsTr("Document ") + tab_view.count(), com_page, colors[Math.floor(Math.random() * 8)]);
    }

    LingmoTabView {
        id: tab_view
        onNewPressed: {
            window.newTab();
        }
    }

    Component{
        id:com_page
        Rectangle{
            anchors.fill: parent
            color: argument.normal
        }
    }
    // ColumnLayout {
    //     anchors.fill: parent

    //     LingmoLoadingButton {
    //         id: button
    //         Layout.alignment: Qt.AlignHCenter
    //         text: "About Qt" // 按钮文本
    //         disabled: false
    //         loading: true

    //         onClicked: {
    //             button.loading = false
    //             aboutQtController.showAboutQtDialog()
    //         }
    //     }

    //     LingmoDropDownBox {
    //         id: dropBox
    //         text: "DropDownButton"
    //         Layout.alignment: Qt.AlignHCenter

    //         LingmoMenuItem {
    //             text: "Menu_1"
    //             onTriggered: {
    //                 console.log("Menu_1 Selected!")
    //                 dropBox.text = text
    //             }
    //         }
    //         LingmoMenuItem {
    //             text: "Menu_2"
    //         }
    //         LingmoMenuItem {
    //             text: "Menu_3"
    //         }
    //         LingmoMenuItem {
    //             text: "Menu_4"
    //         }
    //     }

    //     LingmoSlider {
    //         Layout.alignment: Qt.AlignHCenter
    //     }
    //     LingmoSlider {
    //         Layout.alignment: Qt.AlignHCenter
    //         orientation: Qt.Vertical
    //     }

    //     RowLayout {
    //         spacing: 30
    //         Layout.alignment: Qt.AlignHCenter
    //         LingmoCheckBox {
    //             text: qsTr("Right")
    //         }
    //         LingmoCheckBox {
    //             text: qsTr("Left")
    //             textRight: false
    //         }
    //     }

    //     LingmoTextBox {
    //         Layout.alignment: Qt.AlignHCenter
    //     }

    //     LingmoDelayButton {
    //         Layout.alignment: Qt.AlignHCenter
    //         id: delayBtn
    //         delay: 1000
    //         text: "PressAndHold"

    //         onActivated: {
    //             console.log("成功点击！")
    //         }
    //     }

    //     QQCL.ToolButton {
    //         Layout.alignment: Qt.AlignHCenter
    //         text: "I'm an Button"
    //     }

    //     LingmoFilledIconButton {
    //         id: iconBtn
    //         Layout.alignment: Qt.AlignHCenter
    //         iconSource: LingmoIcons.AddBold
    //         iconSize: 32
    //         implicitWidth: 64
    //         implicitHeight: 64
    //         radius: iconBtn.implicitWidth / 2
    //     }
    // }
}
