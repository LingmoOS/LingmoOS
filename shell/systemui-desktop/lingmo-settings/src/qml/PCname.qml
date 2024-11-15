import QtQuick 2.4
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.0
import LingmoUI 1.0 as LingmoUI

Item {
    id: control

    height: mainLayout.implicitHeight + 1 * 2

    property alias key: keyLabel.text
    property alias value: valueLabel.text

    signal deviceNameChanged(string newDeviceName)

    Layout.fillWidth: true

    ChangeDialog {
        id: pcDialog
    }

    // Dialog {
    //     id: pcDialog
    //     visible: true
    //     title: "Device name"

    //     // width: 350
    //     // height: 250

    //     Text {
    //         id: wsa
    //         anchors.horizontalCenter: parent.horizontalCenter
    //         anchors.verticalCenter: parent.verticalCenter/2
    //         text: qsTr("Enter your new device name")
    //         color: LingmoUI.Theme.textColor
    //         font.pointSize: 15
    //     }

    //     TextField {
    //         id: deviceChange
    //         anchors.horizontalCenter: parent.horizontalCenter
    //         anchors.top: wsa.bottom
    //         anchors.topMargin: 5
    //         width: parent.width - 10
    //         placeholderText: qsTr("New device name")
    //         selectByMouse: true
    //     }

    //     // Button {
    //     //     id: tsd
    //     //     // anchors.bottom: pcDialog.bottom
    //     //     // anchors.bottomMargin: 10
    //     //     // anchors.right: pcDialog.right
    //     //     // anchors.rightMargin: 12
    //     //     anchors {
    //     //         top: pcDialog.bottom
    //     //         topMargin: 10
    //     //         horizontalCenter: parent.horizontalCenter
    //     //     }
    //     //     flat: true
    //     //     text: qsTr("OK")
    //     //     onClicked: {
    //     //         pcDialog.visible = false
    //     //     }
    //     // }
    // }

    Rectangle {
        id: background
        anchors.fill: parent
        color: "transparent"
        radius: LingmoUI.Theme.smallRadius
    }

    RowLayout {
        id: mainLayout
        anchors.fill: parent

        Label {
            id: keyLabel
            color: LingmoUI.Theme.textColor
        }

        Item {
            Layout.fillWidth: true
        }

        Label {
            id: valueLabel
            color: LingmoUI.Theme.disabledTextColor
        }

        Image {
            id: editIcon
            anchors {
                verticalCenter: background.verticalCenter
                // left: pcName.right
                // leftMargin: 2
            }
            // width: 3
            source: LingmoUI.Theme.darkMode ? "qrc:/images/dark/edit.svg" : "qrc:/images/light/edit.svg"

            MouseArea {
                id: changePCname
                anchors.fill:parent
                onClicked: {
                    pcDialog.visible = true
                }
            }
        }
        
    }
}
