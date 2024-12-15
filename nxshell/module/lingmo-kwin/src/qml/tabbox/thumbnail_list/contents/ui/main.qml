import QtQuick 2.0
import QtQuick.Window 2.0
import org.kde.kwin 2.0 as KWin

// https://techbase.kde.org/Development/Tutorials/KWin/WindowSwitcher
KWin.Switcher {
    id: tabBox

    Window {
        id: dialog
        visible: tabBox.visible
        // FramelessWindowHint is needed under wayland platform
        flags: Qt.BypassWindowManagerHint | Qt.FramelessWindowHint
        color: "transparent"

        //NOTE: this is the *current* screen, not the *primary* screen
        x: tabBox.viewRect.x
        y: tabBox.viewRect.y
        width: tabBox.viewRect.width
        height: tabBox.viewRect.height

        // outer border
        Rectangle {
            id: outerBorder

            width: tabBox.viewRect.width
            height: tabBox.viewRect.height
            color: "#19D2D2D2"
            radius: tabBox.windowRadius
            border.width: 1
            border.color: "#19000000"
        }

        // inner border
        Rectangle {
            id: innerBorder
            anchors.fill: parent
            anchors.margins: 1

            color: "transparent"
            radius: outerBorder.radius > 1 ? outerBorder.radius - 1 : 0
            border.width: 1
            border.color: "#19FFFFFF"
        }
    }
}