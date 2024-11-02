import QtQuick 2.12
import QtQuick.Controls 2.15 as Controls
import org.lingmo.quick.items 1.0
import org.lingmo.quick.platform 1.0


MouseArea {
    readonly property alias background: backgroundBase
    property alias text : nameText.text

    hoverEnabled: true
    activeFocusOnTab: true

    StyleBackground {
        id: backgroundBase
        anchors.fill: parent

        border.width: parent.activeFocus ? 2 : 0
        paletteRole: Theme.BrightText
        borderColor: Theme.Highlight
        useStyleTransparency: false
        alpha: parent.containsPress ? 0.30 : parent.containsMouse ? 0.15 : 0.0

        StyleText {
            id: nameText
            width: parent.width
            height: parent.height
            wrapMode: Text.Wrap
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
        }
        Tooltip {
            width: parent.width
            height: parent.height
            mainText: (nameText.text)
            posFollowCursor: false
            location: Dialog.TopEdge
            margin: 9
            interactive: true
        }
    }
}

