import QtQuick
import QtQuick.Controls
import TreeLand.Greeter

Control {
    id: timedataWidget

    property date currentDate: new Date()
    property var currentLocale: Qt.locale()

    Timer {
        interval: 1000
        running: true
        repeat: true
        triggeredOnStart: true

        onTriggered: {
            currentDate = new Date()
        }
    }

    Text {
        id: timeText
        text: currentDate.toLocaleTimeString(currentLocale, "HH:mm")
        font.weight: Font.Light
        font.pixelSize: 80
        font.family: "Source Han Sans CN"
        color: "white"

        anchors.left: parent.left
        anchors.leftMargin: 20
        anchors.top: parent.top
        // anchors.topMargin: 10
    }

    Text {
        id: dateText
        text: currentDate.toLocaleDateString(currentLocale)
        font.family: "Source Han Sans CN"
        font.weight: Font.Normal
        font.pixelSize: 20
        color: "white"

        anchors.left: parent.left
        anchors.leftMargin: 20
        anchors.top: timeText.bottom
    }
}
