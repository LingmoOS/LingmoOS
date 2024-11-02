
import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls.impl 2.12
import QtQuick.Templates 2.12 as T
import org.lingmo.qqc2style.private 1.0 as StylePrivate

T.ApplicationWindow {
    id: window

    color: palette.window
    palette: StylePrivate.StyleHelper.palette
    overlay.modal: Rectangle {
        color: Color.transparent(window.palette.shadow, 0.5)
    }

    overlay.modeless: Rectangle {
        color: Color.transparent(window.palette.shadow, 0.12)
    }

}
