import QtQuick 2.0

Rectangle {
    id: root
    property QtObject effectFrame: null

    width: effectFrame ? effectFrame.size.width : 0
    height: effectFrame ? effectFrame.size.height : 0
    color: "transparent"

    Image {
        id: icon
        anchors.fill: parent
        sourceSize: effectFrame ? effectFrame.size : Qt.size(0, 0)
        source: effectFrame ? effectFrame.image : ""
    }
}