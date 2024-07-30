import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.lingmoui as LingmoUI

Item {
    id: control

    height: mainLayout.implicitHeight + 1 * 2

    property alias key: keyLabel.text
    property alias value: valueLabel.text

    Layout.fillWidth: true

    Rectangle {
        id: background
        anchors.fill: parent
        color: "transparent"
        radius: 11
    }

    RowLayout {
        id: mainLayout
        anchors.fill: parent

        LingmoUI.Label {
            id: keyLabel
            // color: LingmoUI.Theme.textColor
        }

        Item {
            Layout.fillWidth: true
        }

        LingmoUI.Label {
            id: valueLabel
            color: LingmoUI.Theme.disabledTextColor
        }
    }
}
