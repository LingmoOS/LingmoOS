import QtQuick
import QtQuick.Layouts
import TreeLand.Greeter
import org.lingmo.dtk 1.0 as D
import QtQuick.Controls

D.Popup {
    property string hintText
    modal: true
    background: D.FloatingPanel {
        anchors.fill: parent
        radius: 12
        blurRadius: 12
        backgroundColor: D.Palette {
            normal: Qt.rgba(238 / 255, 238 / 255, 238 / 255, 0.3)
        }
    }

    ColumnLayout {
        Layout.alignment: Qt.AlignLeft
        Text {
            text: qsTr("Password Hint")
            color: Qt.rgba(1, 1, 1, 0.7)
            font.weight: Font.Medium
            font.pixelSize: 10
            font.family: "Source Han Sans CN"
        }
        Text {
            text: hintText
            color: "white"
            font.weight: Font.Medium
            font.pixelSize: 12
            font.family: "Source Han Sans CN"
        }
    }
}
