import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.12
Rectangle {
    id: root
    objectName: "textEditPage"
    visible: true
    color: "transparent"
    property var imageText
    anchors.fill: imageText
    scale: imageText.scale
    rotation: imageText.rotation
    //鼠标点击坐标位置---画矩形
    property real startX:-1     //储存鼠标开始时的坐标
    property real startY:-1
    property real stopX      //储存鼠标结束时的坐标
    property real stopY
    property bool isMouseMoveEnable: false //是否允许鼠标移动绘制事件
    property bool hasPaint: false

    Rectangle{
        id:maskRect
        anchors.fill: parent
        color:"transparent"
        //        visible: false

        TextEdit {
            id: textInput
            wrapMode: TextEdit.Wrap
            font.bold: variables.isBold
            font.italic: variables.isItalics
            font.pointSize: variables.fontSize
            font.family: variables.fontType
            font.underline: variables.isUnderLine
            font.strikeout: variables.isDeleteLine
            selectByMouse:true
            focus: true
            clip: true
            color: variables.painterColor

            onTextChanged: {
                if (textInput.text == "") {
                    hasPaint = false
                } else {
                    hasPaint = true
                }
            }
        }
        MouseArea {
            id: textRectArea
            anchors.fill: parent
            onPressed: {
                //每次按下时，需要重新创建一个text画布
                if (hasPaint) {
                    sendTextOperate()
                    variables.textMouseX = mouseX
                    variables.textMouseY = mouseY
                    textInput.focus = false
                    textInput.cursorVisible = false
                    variables.painterTextChanged(mouseX,mouseY)
                    return;
                }
                setProptyReact(mouseX,mouseY)
                startX = mouseX
                startY = mouseY
                variables.textMouseX = mouseX
                variables.textMouseY = mouseY
            }
        }
    }

    function clearMask (){
        root.destroy()
    }
    function acturalType() {
        return root.objectName
    }
    function setProptyReact(actualMouseX,actualMouseY) {
        textInput.x = actualMouseX
        textInput.y = actualMouseY
        textInput.forceActiveFocus()
        textInput.cursorVisible = true
    }
    function sendTextOperate() {
        if (textInput.text == "" ) {
            return
        }
        variables.painterTextPaint(textInput.text,Qt.point(variables.textMouseX,variables.textMouseY),6)
        //属性解绑
        attributeUnbinding()
    }
    function attributeUnbinding() {

        var bold = variables.isBold
        var italic= variables.isItalics
        var  pointSize= variables.fontSize
        var family= variables.fontType
        var underline= variables.isUnderLine
        var strikeout= variables.isDeleteLine
        var textColor =variables.painterColor

        textInput.font.bold= bold
        textInput.font.italic= italic
        textInput.font.pointSize= pointSize
        textInput.font.family= family
        textInput.font.underline= underline
        textInput.font.strikeout= strikeout
        textInput.color= textColor
    }
}


