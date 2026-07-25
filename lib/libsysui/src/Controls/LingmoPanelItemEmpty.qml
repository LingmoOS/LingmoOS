import QtQuick
import QtQuick.Controls
import LingmoUI

QtObject {
    property string key
    property int _idx
    property var _ext
    property var _parent
    property bool visible: true
    Component.onCompleted: {
        key = LingmoTools.uuid()
    }
}
