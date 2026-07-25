// A wraper for text
import QtQuick
import QtQuick.Controls
import LingmoUI

Text {
    property color textColor: LingmoTheme.fontPrimaryColor
    id:text
    color: enabled ? textColor : (LingmoTheme.dark ? Qt.rgba(131/255,131/255,131/255,1) : Qt.rgba(160/255,160/255,160/255,1))
    renderType: LingmoTheme.nativeText ? Text.NativeRendering : Text.QtRendering
    font: LingmoTextStyle.Body
}
