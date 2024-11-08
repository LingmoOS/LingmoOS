import QtQuick 2.15
import org.lingmo.menu.core 1.0

import org.lingmo.quick.items 1.0 as LingmoUIItems
import org.lingmo.quick.platform 1.0 as Platform

MouseArea {
    id: buttonMouseArea
    hoverEnabled: true
    property string buttonIcon: ""
    property alias highlight: themeIcon.highLight
    property alias autoHighLight: themeIcon.autoHighLight

    LingmoUIItems.StyleBackground {
        id: buttonBase
        useStyleTransparency: false
        paletteRole: Platform.Theme.Text
        anchors.fill: parent
        radius: height / 2
        alpha: buttonMouseArea.containsPress ? 0.20 : buttonMouseArea.containsMouse ? 0.16 : 0.10
    }

    LingmoUIItems.Icon {
        id: themeIcon
        anchors.centerIn: parent
        width: 16; height: width
        source: buttonIcon
        mode: LingmoUIItems.Icon.AutoHighlight
    }
    states: State {
        when: buttonMouseArea.activeFocus
        PropertyChanges {
            target: buttonBase
            borderColor: Platform.Theme.Highlight
            border.width: 2
        }
    }
}
