import QtQuick 2.12
import org.lingmo.quick.items 1.0
import org.lingmo.quick.platform 1.0
Column {
    property alias paletteRole: backGround.paletteRole
    property alias paletteGroup: backGround.paletteGroup
    property alias colorText: colorText.text
    width: childrenRect.width
    StyleBackground {
        id: backGround
        width: 120
        height: 80
        radius: 0
        paletteRole: Theme.BrightText
        useStyleTransparency: false
        alpha: 1
    }
    StyleText {
        id: colorText
        wrapMode: Text.Wrap
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter
    }
}
