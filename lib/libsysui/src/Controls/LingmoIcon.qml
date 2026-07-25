import QtQuick
import QtQuick.Controls
import LingmoUI

Text {
    property int iconSource
    property int iconSize: 20
    property color iconColor: {
        if(LingmoTheme.dark){
            if(!enabled){
                return Qt.rgba(130/255,130/255,130/255,1)
            }
            return Qt.rgba(1,1,1,1)
        }else{
            if(!enabled){
                return Qt.rgba(161/255,161/255,161/255,1)
            }
            return Qt.rgba(0,0,0,1)
        }
    }
    id: control
    font.family: font_loader.name
    font.pixelSize: iconSize
    horizontalAlignment: Text.AlignHCenter
    verticalAlignment: Text.AlignVCenter
    color: iconColor
    text: (String.fromCharCode(iconSource).toString(16))
    opacity: iconSource > 0
    FontLoader {
        id: font_loader
        source: "qrc:/lingmoui/kit/LingmoUI/Font/FluentIcons.ttf"
    }
}
