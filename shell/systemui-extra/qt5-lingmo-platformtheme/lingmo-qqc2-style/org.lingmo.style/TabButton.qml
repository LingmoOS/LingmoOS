
import QtQuick 2.6
import QtQml.Models 2.1
import QtQuick.Controls 2.5
import org.lingmo.qqc2style.private 1.0 as StylePrivate
import QtQuick.Templates 2.5 as T

T.TabButton {
    id: controlRoot
    palette: StylePrivate.StyleHelper.palette
    font: StylePrivate.StyleHelper.font

    //Some qstyles like fusion don't have correct pixel metrics here and just return 0
    implicitWidth: Math.max(styleitem.implicitWidth, textMetrics.width + textMetrics.height * 2)
    implicitHeight: styleitem.implicitHeight || textMetrics.height * 2
    baselineOffset: contentItem.y + contentItem.baselineOffset

    //This width: is important to make the tabbar internals not assume
    //all tabs have the same width
    width: implicitWidth
    padding: 0

    hoverEnabled: true

    contentItem: Item {}


    background: StylePrivate.StyleItem {
        id: styleitem

        TextMetrics {
            id: textMetrics

            text: controlRoot.text
        }

        control: controlRoot
        anchors.fill: parent
        elementType: "tab"
        paintMargins: 0

        properties: {
            "hasFrame" : true,
            "icon": control.icon ? (control.icon.name || control.icon.source) : "",
            "iconWidth": controlRoot.icon && controlRoot.icon.width ? controlRoot.icon.width : 0,
            "iconHeight": controlRoot.icon && controlRoot.icon.height ? controlRoot.icon.height : 0,
            "flat": controlRoot.flat

        }


        enabled: controlRoot.enabled
        selected: controlRoot.checked
        text: controlRoot.text
        hover: controlRoot.hovered
        hasFocus: controlRoot.activeFocus
    }
}
