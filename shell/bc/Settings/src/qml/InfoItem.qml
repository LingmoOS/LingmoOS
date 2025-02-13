import QtQuick 2.4
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.3
import LingmoUI 1.0 as LingmoUI

Item {
    id: control

    height: mainLayout.implicitHeight + 1 * 2

    // property alias icon: iconLable.icon
    property alias key: keyLabel.text
    property alias value: valueLabel.text

    // Layout.fillWidth: true

    Rectangle {
        id: background
        anchors.fill: parent
        color: "transparent"
        radius: LingmoUI.Theme.smallRadius
    }

    ColumnLayout {
        id: mainLayout
        anchors.fill: parent

        // Lable {
        //     id: image
        //     Image {
        //                         id: displayIcon
        //                         anchors {
        //                             top: display.top
        //                             topMargin: 10
        //                             // verticalCenter: parent.verticalCenter
        //                         }
        //                         width: 30
        //                         sourceSize: Qt.size(width, height)
        //                         source: "qrc:/images/sidebar/all/display.svg"
        //                     }
        // }

        Label {
            id: keyLabel
            color: LingmoUI.Theme.textColor
        }

        // Item {
        //     Layout.fillWidth: true
        // }

        Label {
            id: valueLabel
            color: LingmoUI.Theme.disabledTextColor
        }
    }
}
