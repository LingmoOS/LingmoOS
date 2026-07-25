import QtQuick
import QtQuick.Window
import LingmoUI

Item {
    id:control
    property int orientation: Qt.Horizontal
    property int spacing:0
    property int size: 1
    QtObject{
        id:d
        property bool isVertical : orientation === Qt.Vertical
        property int parentHeight: {
            if(control.parent){
                return control.parent.height
            }
            return control.height
        }
        property int parentWidth: {
            if(control.parent){
                return control.parent.width
            }
            return control.width
        }
    }
    width: d.isVertical ? spacing*2+size : d.parentWidth
    height: d.isVertical ? d.parentHeight : spacing*2+size
    LingmoRectangle{
        color: LingmoTheme.dividerColor
        width: d.isVertical ? size : control.width
        height: d.isVertical ? control.height : size
        anchors.centerIn: parent
    }
}
