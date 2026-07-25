import QtQuick
import QtQuick.Controls
import QtQuick.Controls.impl
import QtQuick.Templates as T
import QtQuick.Window
import LingmoUI

T.Menu {
    property bool animationEnabled: true
    id: control
    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            contentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             contentHeight + topPadding + bottomPadding)
    margins: 0
    overlap: 1
    spacing: 0
    delegate: LingmoMenuItem {}
    enter: Transition {
        NumberAnimation {
            property: "opacity"
            from: 0
            to: 1
            duration: LingmoTheme.animationEnabled
                      && control.animationEnabled ? 83 : 0
        }
    }
    exit: Transition {
        NumberAnimation {
            property: "opacity"
            from: 1
            to: 0
            duration: LingmoTheme.animationEnabled
                      && control.animationEnabled ? 83 : 0
        }
    }
    contentItem: ListView {
        implicitHeight: contentHeight
        model: control.contentModel
        interactive: Window.window ? contentHeight + control.topPadding
                                     + control.bottomPadding > Window.window.height : false
        clip: true
        currentIndex: control.currentIndex
        ScrollBar.vertical: LingmoScrollBar {}
    }
    background: Rectangle {
        implicitWidth: 150
        implicitHeight: 36
        color: LingmoTheme.dark ? Qt.rgba(
                                      45 / 255, 45 / 255, 45 / 255,
                                      1) : Qt.rgba(252 / 255, 252 / 255, 252 / 255, 1)
        border.color: LingmoTheme.dark ? Qt.rgba(
                                             26 / 255, 26 / 255, 26 / 255,
                                             1) : Qt.rgba(191 / 255, 191 / 255, 191 / 255, 1)
        border.width: 1
        radius: LingmoUnits.smallRadius + 1
        LingmoShadow {
            radius: LingmoUnits.smallRadius
        }
    }
    T.Overlay.modal: Rectangle {
        color: LingmoTools.withOpacity(control.palette.shadow, 0.5)
        radius: window.windowVisibility === Window.Maximized ? 0 : LingmoUnits.windowRadius
    }
    T.Overlay.modeless: Rectangle {
        color: LingmoTools.withOpacity(control.palette.shadow, 0.12)
        radius: window.windowVisibility === Window.Maximized ? 0 : LingmoUnits.windowRadius
    }
}
