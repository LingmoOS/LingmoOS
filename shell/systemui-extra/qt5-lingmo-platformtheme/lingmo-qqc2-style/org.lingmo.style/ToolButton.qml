import QtQuick.Controls 2.5
import QtQuick 2.6
import QtQuick.Templates 2.5 as T
import org.lingmo.qqc2style.private 1.0 as StylePrivate

T.ToolButton {
    id: controlRoot

    palette: StylePrivate.StyleHelper.palette
    font: StylePrivate.StyleHelper.font

    property string icontype: "default"

    implicitWidth: text.length > 0 ? background.implicitWidth : implicitHeight
    implicitHeight: background.implicitHeight

    hoverEnabled: true //Qt.styleHints.useHoverEffects TODO: how to make this work in 5.7?

    flat: true
    contentItem: Item {}

    StylePrivate.KyIcon {
        id:icon
        iconName: controlRoot.icon.name
        anchors.verticalCenter: parent.verticalCenter
        width: controlRoot.icon.width
        height: controlRoot.icon.height
        sunken: controlRoot.pressed || (controlRoot.checkable && controlRoot.checked)
        hover: controlRoot.hovered
        hasFocus: controlRoot.activeFocus
        icontype: controlRoot.icontype
    }

    background: StylePrivate.StyleItem {
        id: styleitem
        anchors.fill: parent
        control: controlRoot
        elementType: controlRoot.flat ? "toolbutton" : "button"
        on: controlRoot.pressed || (controlRoot.checkable && controlRoot.checked)
        hover: controlRoot.hovered
        text: controlRoot.text
        hasFocus: false
        activeControl: controlRoot.isDefault ? "default" : "f"
    }
}
