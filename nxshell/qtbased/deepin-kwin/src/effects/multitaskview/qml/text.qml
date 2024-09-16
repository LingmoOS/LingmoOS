import QtQuick 2.0
import QtQuick.Layouts 1.0

ColumnLayout {
    id: root
    property QtObject effectFrame: null
    Text {
        font.family: effectFrame ? effectFrame.font.family : ""
        font.pointSize: effectFrame ? effectFrame.font.pointSize : 10
        text: effectFrame ? effectFrame.text : ""
    }
}