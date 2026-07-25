import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LingmoUI

LingmoWindow {

    id: window
    property string sequence: ""
    title: qsTr("Hotkey Show")
    width: 250
    height: 250
    fixSize: true
    launchMode: LingmoWindowType.SingleInstance
    onInitArgument:
        (argument)=>{
            window.sequence = argument.sequence
        }
    LingmoText{
        anchors.centerIn: parent
        color: LingmoTheme.primaryColor
        font: LingmoTextStyle.Title
        text: window.sequence
    }
}
