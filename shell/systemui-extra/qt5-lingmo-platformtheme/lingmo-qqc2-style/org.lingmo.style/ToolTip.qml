
import QtQuick 2.6
import QtGraphicalEffects 1.0
import QtQuick.Controls 2.5 as Controls
import QtQuick.Templates 2.5 as T
import org.lingmo.qqc2style.private 1.0 as StylePrivate


T.ToolTip {
    id: controlRoot

    palette: StylePrivate.StyleHelper.palette
    font: StylePrivate.StyleHelper.font


    x: parent ? Math.round((parent.width - implicitWidth) / 2) : 0
    y: -implicitHeight - 3

    // Always show the tooltip on top of everything else
    z: 999

    implicitWidth: contentItem.implicitWidth + leftPadding + rightPadding
    implicitHeight: contentItem.implicitHeight + topPadding + bottomPadding

    margins: 6
    padding: 6


    // Timeout based on text length, from QTipLabel::restartExpireTimer
    timeout: 10000 + 40 * Math.max(0, text.length - 100)

    closePolicy: T.Popup.CloseOnEscape | T.Popup.CloseOnPressOutsideParent | T.Popup.CloseOnReleaseOutsideParent

    contentItem: Controls.Label {
        text: controlRoot.text
        wrapMode: Text.WordWrap
        font: controlRoot.font
        color: controlRoot.palette.windowText
    }


    background: Rectangle {
        radius: 3

        //border.color: controlRoot.palette.dark
        color: controlRoot.palette.toolTipBase

        layer.enabled: true
        layer.effect: DropShadow {
            transparentBorder: true
            radius: 4
            samples: 8
            horizontalOffset: 0
            verticalOffset: 0
            color: Qt.rgba(0, 0, 0, 0.3)

        }
    }

}
