
import QtQuick 2.6
import QtGraphicalEffects 1.0
import QtQuick.Templates 2.5 as T
import org.lingmo.qqc2style.private 1.0 as StylePrivate

T.Popup {
    id: control

    palette: StylePrivate.StyleHelper.palette
    font: StylePrivate.StyleHelper.font

    implicitWidth: Math.max(background ? background.implicitWidth : 0,
                            contentWidth > 0 ? contentWidth + leftPadding + rightPadding : 0)
    implicitHeight: Math.max(background ? background.implicitHeight : 0,
                             contentWidth > 0 ? contentHeight + topPadding + bottomPadding : 0)

    contentWidth: contentItem.implicitWidth || (contentChildren.length === 1 ? contentChildren[0].implicitWidth : 0)
    contentHeight: contentItem.implicitHeight || (contentChildren.length === 1 ? contentChildren[0].implicitHeight : 0)

    padding: 12
    clip: true

    enter: Transition {
        NumberAnimation {
            property: "opacity"
            from: 0
            to: 1
            easing.type: Easing.InOutQuad
            duration: 250
        }
    }

    exit: Transition {
        NumberAnimation {
            property: "opacity"
            from: 1
            to: 0
            easing.type: Easing.InOutQuad
            duration: 250
        }
    }

    contentItem: Item { }

    background: Rectangle {
        radius: 2
        color: control.palette.window
        border.color: control.palette.dark
        layer.enabled: true

        layer.effect: DropShadow {
            transparentBorder: true
            radius: 8
            samples: 16
            horizontalOffset: 0
            verticalOffset: 4
            color: Qt.rgba(0, 0, 0, 0.3)
        }
    }
}
