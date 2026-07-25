import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Window
import LingmoUI

Page {
    property int launchMode: LingmoPageType.SingleTop
    property bool animationEnabled: LingmoTheme.animationEnabled
    property string url : ""
    id: control
    StackView.onRemoved: destroy()
    padding: 5
    visible: false
    opacity: visible
    transform: Translate {
        y: control.visible ? 0 : 80
        Behavior on y{
            enabled: control.animationEnabled && LingmoTheme.animationEnabled
            NumberAnimation{
                duration: 167
                easing.type: Easing.OutCubic
            }
        }
    }
    Behavior on opacity {
        enabled: control.animationEnabled && LingmoTheme.animationEnabled
        NumberAnimation{
            duration: 83
        }
    }
    background: Item{}
    header: LingmoLoader{
        sourceComponent: control.title === "" ? undefined : com_header
    }
    Component{
        id: com_header
        Item{
            implicitHeight: 40
            LingmoText{
                id:text_title
                text: control.title
                font: LingmoTextStyle.Title
                anchors{
                    left: parent.left
                    leftMargin: 5
                }
            }
        }
    }
    Component.onCompleted: {
        control.visible = true
    }
}
