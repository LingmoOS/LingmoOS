import QtQuick 2.1
import QtQuick.Window 2.2
import QtQuick.Templates 2.3 as T
import LingmoUI 1.0 as LingmoUI

T.Label {
    id: control

    verticalAlignment: lineCount > 1 ? Text.AlignTop : Text.AlignVCenter

    activeFocusOnTab: false
    // Text.NativeRendering is broken on non integer pixel ratios
    // renderType: Window.devicePixelRatio % 1 !== 0 ? Text.QtRendering : Text.NativeRendering

    renderType: LingmoUI.Theme.renderType

    font.capitalization: LingmoUI.Theme.defaultFont.capitalization
    font.family: LingmoUI.Theme.fontFamily
    font.italic: LingmoUI.Theme.defaultFont.italic
    font.letterSpacing: LingmoUI.Theme.defaultFont.letterSpacing
    font.pointSize: LingmoUI.Theme.fontSize
    font.strikeout: LingmoUI.Theme.defaultFont.strikeout
    font.underline: LingmoUI.Theme.defaultFont.underline
    font.weight: LingmoUI.Theme.defaultFont.weight
    font.wordSpacing: LingmoUI.Theme.defaultFont.wordSpacing
    color: LingmoUI.Theme.textColor
    linkColor: LingmoUI.Theme.linkColor

    opacity: enabled ? 1 : 0.6

    Accessible.role: Accessible.StaticText
    Accessible.name: text
}
