import QtQuick 2.0

Rectangle {
    id: root
    property QtObject effectFrame: null

    width: effectFrame ? effectFrame.size.width : 0
    height: effectFrame ? effectFrame.size.height : 0
    radius: effectFrame ? effectFrame.radius : 0
    color: "transparent"

    border.color: effectFrame ? effectFrame.color : ""
    border.width: 4;
}
