
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls.impl 2.12
import QtQuick.Templates 2.12 as T
import org.lingmo.qqc2style.private 1.0 as StylePrivate

T.ScrollBar {
    id: control

    palette: StylePrivate.StyleHelper.palette
    font: StylePrivate.StyleHelper.font


    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                                implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                                 implicitContentHeight + topPadding + bottomPadding)

    hoverEnabled: true



    padding: 2
    visible: control.policy !== T.ScrollBar.AlwaysOff



    contentItem: Rectangle {
        implicitWidth: control.interactive ? 6 : 2
        implicitHeight: control.interactive ? 6 : 2

        radius: width / 2
        color: control.palette.shadow

        opacity: 0.3


        states: State {
            name: "active"
            when: control.policy === T.ScrollBar.AlwaysOn || (control.active && control.size < 1.0)
            PropertyChanges { target: control.contentItem; implicitWidth:8.5; implicitHeight:8.5; opacity:0.5}
        }

        transitions: Transition {
            from: "active"
            SequentialAnimation {
                PauseAnimation { duration: 200 }
                NumberAnimation { target: control.contentItem; duration: 80; property: "opacity"; to: 0.3}
            }
        }
    }

}


