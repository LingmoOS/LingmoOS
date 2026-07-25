import QtQuick
import QtQuick.Controls
import LingmoUI

QtObject {
    property string key
    property int _idx
    property bool visible: true
    property var parent
    property real spacing
    property int size:1
    Component.onCompleted: {
        key = LingmoTools.uuid()
    }
}
