import QtQuick 2.0
import QtQuick.Controls 2.5
import org.lingmo.menu.core 1.0
import org.lingmo.quick.items 1.0 as LingmoUIItems
import org.lingmo.quick.platform 1.0 as Platform

ScrollBar {
    id: control
    padding: (hovered || pressed) ? 3 : 5
    property bool visual: true

    Behavior on padding {
        NumberAnimation {
            duration: 200
            easing.type: Easing.InOutQuad
        }
    }

    contentItem: LingmoUIItems.StyleBackground {
        radius: width / 2
        useStyleTransparency: false
        paletteRole: Platform.Theme.Text
        alpha: control.pressed ? 0.28 : control.hovered ? 0.18 : 0.10
        opacity: ((control.policy === ScrollBar.AlwaysOn || control.size < 1.0 ) && control.visual) ? 1.0 : 0.0
    }
}
