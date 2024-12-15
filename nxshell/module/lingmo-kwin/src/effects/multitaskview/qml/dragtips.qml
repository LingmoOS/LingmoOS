import QtQuick 2.0
import QtQuick.Layouts 1.0

RowLayout {
    id: root
    property QtObject effectFrame: null

    spacing: 13

    Image {
        id: icon
        Layout.preferredWidth: effectFrame ? effectFrame.iconSize.width : 0
        Layout.preferredHeight: effectFrame ? effectFrame.iconSize.height : 0
        source: effectFrame ? effectFrame.image : ""
    }

    Text {
        id: label
        font.family: effectFrame ? effectFrame.font.family : ""
        font.pointSize: effectFrame ? effectFrame.font.pointSize : 10
        text: effectFrame ? effectFrame.text : ""
        color: effectFrame ? effectFrame.color : ""
    }
}