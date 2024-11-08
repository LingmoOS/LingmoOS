import QtQuick 2.4
import QtQuick.Controls 2.4
import QtGraphicalEffects 1.0
import LingmoUI 1.0 as LingmoUI

Item {
    id: control

    property alias source: image.source
    signal clicked()

    width: 22
    height: width

    Image {
        id: image
        anchors.fill: parent
        sourceSize: Qt.size(control.width, control.height)
        smooth: false
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton
        onClicked: control.clicked()
    }

    ColorOverlay {
        anchors.fill: image
        source: image
        color: "white"
        opacity: 0.5
        visible: mouseArea.containsPress
    }
}
