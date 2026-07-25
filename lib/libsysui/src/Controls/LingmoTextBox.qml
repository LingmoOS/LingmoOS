import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Controls.Basic
import LingmoUI

TextField{
    signal commit(string text)
    property bool disabled: false
    property int iconSource: 0
    property color normalColor: LingmoTheme.dark ?  Qt.rgba(255/255,255/255,255/255,1) : Qt.rgba(27/255,27/255,27/255,1)
    property color disableColor: LingmoTheme.dark ? Qt.rgba(131/255,131/255,131/255,1) : Qt.rgba(160/255,160/255,160/255,1)
    property color placeholderNormalColor: LingmoTheme.dark ? Qt.rgba(210/255,210/255,210/255,1) : Qt.rgba(96/255,96/255,96/255,1)
    property color placeholderFocusColor: LingmoTheme.dark ? Qt.rgba(152/255,152/255,152/255,1) : Qt.rgba(141/255,141/255,141/255,1)
    property color placeholderDisableColor: LingmoTheme.dark ? Qt.rgba(131/255,131/255,131/255,1) : Qt.rgba(160/255,160/255,160/255,1)
    property bool cleanEnabled: true
    id:control
    padding: 7
    leftPadding: padding + 4
    enabled: !disabled
    color: {
        if(!enabled){
            return disableColor
        }
        return normalColor
    }
    font:LingmoTextStyle.Body
    renderType: LingmoTheme.nativeText ? Text.NativeRendering : Text.QtRendering
    selectionColor: LingmoTools.withOpacity(LingmoTheme.primaryColor,0.5)
    selectedTextColor: color
    placeholderTextColor: {
        if(!enabled){
            return placeholderDisableColor
        }
        if(focus){
            return placeholderFocusColor
        }
        return placeholderNormalColor
    }
    selectByMouse: true
    rightPadding: {
        var w = 30
        if(control.cleanEnabled === false){
            w = 0
        }
        if(control.readOnly)
            w = 0
        return icon_end.visible ? w+36 : w+10
    }
    width: 240
    background: LingmoTextBoxBackground{
        inputItem: control
    }
    Keys.onEnterPressed: (event)=> d.handleCommit(event)
    Keys.onReturnPressed:(event)=> d.handleCommit(event)
    QtObject{
        id:d
        function handleCommit(event){
            control.commit(control.text)
        }
    }
    MouseArea{
        anchors.fill: parent
        cursorShape: Qt.IBeamCursor
        acceptedButtons: Qt.RightButton
        onClicked: {
            if(control.echoMode === TextInput.Password){
                return
            }
            if(control.readOnly && control.text === ""){
                return
            }
            menu_loader.popup()
        }
    }
    RowLayout{
        height: parent.height
        anchors{
            right: parent.right
            rightMargin: 5
        }
        spacing: 4
        LingmoIconButton{
            iconSource: LingmoIcons.Cancel
            iconSize: 12
            Layout.preferredWidth: 30
            Layout.preferredHeight: 20
            Layout.alignment: Qt.AlignVCenter
            iconColor: LingmoTheme.dark ? Qt.rgba(222/255,222/255,222/255,1) : Qt.rgba(97/255,97/255,97/255,1)
            verticalPadding: 0
            horizontalPadding: 0
            visible: {
                if(control.cleanEnabled === false){
                    return false
                }
                if(control.readOnly)
                    return false
                return control.text !== ""
            }
            contentDescription:"Clean"
            onClicked:{
                control.clear()
            }
        }
        LingmoIcon{
            id:icon_end
            iconSource: control.iconSource
            iconSize: 12
            Layout.alignment: Qt.AlignVCenter
            Layout.rightMargin: 7
            iconColor: LingmoTheme.dark ? Qt.rgba(222/255,222/255,222/255,1) : Qt.rgba(97/255,97/255,97/255,1)
            visible: control.iconSource != 0
        }
    }
    LingmoLoader{
        id: menu_loader
        function popup(){
            sourceComponent = menu
        }
    }
    Component{
        id:menu
        LingmoTextBoxMenu{
            inputItem: control
            Component.onCompleted: {
                popup()
            }
            onClosed: {
                menu_loader.sourceComponent = undefined
            }
        }
    }
}
