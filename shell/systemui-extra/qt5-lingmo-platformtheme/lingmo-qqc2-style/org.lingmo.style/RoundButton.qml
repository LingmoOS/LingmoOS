
import QtQuick 2.6
import QtQuick.Templates 2.5 as T
import org.lingmo.qqc2style.private 1.0 as StylePrivate

T.RoundButton {
    id: controlRoot
    palette: StylePrivate.StyleHelper.palette

    /* The value type of buttonType are "CloseButton","MaxButton","MinButton","blueButton","Default". */
    StylePrivate.StyleHelper.buttonType: "Default"


    implicitWidth: background.implicitWidth
    implicitHeight: background.implicitHeight

    hoverEnabled: true //Qt.styleHints.useHoverEffects TODO: how to make this work in 5.7?

    contentItem: Item {}

    background: StylePrivate.StyleItem {
        id: styleitem
        anchors.fill: parent
        buttonType: controlRoot.StylePrivate.StyleHelper.buttonType
        control: controlRoot
        elementType: "button"
        roundButton:"RoundButton"
        sunken: controlRoot.pressed || (controlRoot.checkable && controlRoot.checked)
        raised: !(controlRoot.pressed || (controlRoot.checkable && controlRoot.checked))
        hover: controlRoot.hovered
        text: controlRoot.text
        hasFocus: controlRoot.activeFocus
        activeControl: controlRoot.isDefault ? "default" : "f"
        properties: {
            "icon": controlRoot.icon && controlRoot.display !== T.AbstractButton.TextOnly ? (controlRoot.icon.name || controlRoot.icon.source) : "",
            "iconWidth": controlRoot.icon && controlRoot.icon.width ? controlRoot.icon.width : 0,
            "iconHeight": controlRoot.icon && controlRoot.icon.height ? controlRoot.icon.height : 0,
            "flat": controlRoot.flat
        }
    }
}


