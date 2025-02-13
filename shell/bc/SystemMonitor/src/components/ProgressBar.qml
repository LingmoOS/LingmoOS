import QtQuick 2.15
import QtQuick.Controls 2.15
import LingmoUI 1.0 as LingmoUI

ProgressBar {
    id: control
    
    background: Rectangle {
        implicitWidth: 200
        implicitHeight: 6
        color: LingmoUI.Theme.backgroundColor
        radius: height / 2
    }

    contentItem: Item {
        Rectangle {
            width: control.visualPosition * parent.width
            height: parent.height
            radius: height / 2
            color: LingmoUI.Theme.highlightColor

            Behavior on width {
                NumberAnimation {
                    duration: 300
                    easing.type: Easing.OutCubic
                }
            }
        }
    }
} 