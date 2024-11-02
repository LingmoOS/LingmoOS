/*
* Copyright 2022 KylinSoft Co., Ltd.
*
* This program is free software: you can redistribute it and/or modify it under
* the terms of the GNU General Public License as published by the Free Software
* Foundation, either version 3 of the License, or (at your option) any later
* version.
*
* This program is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
* FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
* details.
*
* You should have received a copy of the GNU General Public License along with
* this program. If not, see <https://www.gnu.org/licenses/>.
*/

import QtQuick 2.0
import "js/utils.js" as Utils
import QtQuick.Window 2.0

Rectangle {
    id: virtualKeyboard
    property bool isFloatMode : view.isFloatMode
    anchors.fill: parent
    color: virtualKeyboardColor
    radius: isFloatMode ? virtualKeyboardFloatPlacementRadius :  0

    //候选词
    property var candidateList
    //候选列表高亮候选的索引
    property int globalCursorIndex : -1
    //可用输入法列表
    property var currentIMList : model.currentIMList
    //当前输入法名称
    property string uniqueName : model.uniqueName
    //当前输入法，组成为"uniqueName|localName|label"
    //TOOD 目前以“|”分割数据
    property string currentIM: "||"

    //大小相关
    /*设置默认值，防止报错，fontSize为0时会报错，加载qml文件时height为0，会导致fontSize为0*/
    property real cardinalNumber: height == 0 ? 8 : height/(isFloatMode ? 68.5 : 64)
    property int dragBarHeight: cardinalNumber * 4.5
    property int preeditHeight: cardinalNumber * 5
    property int toolAndCandidateHeight: cardinalNumber * 8
    property int keyboardLayoutHeight: cardinalNumber * 49
    property int keySpacing: cardinalNumber
    property int keyWidth: (virtualKeyboardContent.width - 5*cardinalNumber) * 2/31 - cardinalNumber
    /*暂时删除win键，将空格键长度从keyWidth * 5.5 + keySpacing * 5修改为keyWidth * 6.5 + keySpacing * 6*/
    property int spaceKeyWidth: keyWidth * 6.5 + keySpacing * 6
    property int keyHeight: cardinalNumber * 9
    property int firstRowKeyHeight: keyHeight * 7/9
    property int keyLableAlignment: keyWidth / 3
    property int keyIconAlignment: keyWidth / 3
    property int virtualKeyboardAvailableHeight: isFloatMode ? virtualKeyboard.height - dragBar.height
                                                                 : virtualKeyboard.height
    property int imListItemHeight: virtualKeyboardAvailableHeight * 1/10
    property int imListItemWidth: virtualKeyboardAvailableHeight * 5/13
    property real fontSize: keyHeight * 6/11 * 7/12
    property real shiftFontSize: fontSize / 2
    property real switchKeyFontSize: fontSize * 3/4
    property real fnSymbolKeyFontSize: fontSize * 3/4
    property real actionKeyFontSize: fontSize * 3/4
    property real preeditTextFontSize: fontSize * 3/4
    property real candidateFontSize: fontSize * 3/4
    property real imListFontSize: fontSize * 3/4
    property int preeditX: cardinalNumber * 3.5
    property int candidateListWidth: virtualKeyboardContent.width - 7*cardinalNumber
    property int toolbarSize: cardinalNumber * 3
    property int candidateListX: cardinalNumber * 2
    property int candidateListSpacing: cardinalNumber * 3
    property int keyRadius: cardinalNumber
    property int longPressInterval: 1000
    property int shiftLeftMargin: cardinalNumber
    property int shiftTopMargin: cardinalNumber/2
    property int imLeftMargin: cardinalNumber
    property int dropShadowVerticalOffset: cardinalNumber/4
    property int dragBarIndicatorWidth: cardinalNumber * 7
    property int dragBarIndicatorHeight: cardinalNumber/2

    //颜色相关
    property color virtualKeyboardColor: "#EBEDEF"   /*虚拟键盘背景色*/
    property color charKeyNormalColor: "#FFFFFF"    /*字符按键默认态颜色*/
    property color charKeyPressedColor: "#DDE0E4"   /*字符按键按下态颜色*/
    property color charKeyHoverColor: "#EDF1F5"    /*字符按键悬浮态颜色*/
    property color charKeyDropShadowColor: "#95A0AD"    /*字符按键偏移色*/

    property color actionKeyNormalColor: "#CED3D9"     /*动作按键默认态*/
    property color actionKeyPressedColor: "#A5ACB5"    /*动作按键按下态*/
    property color actionKeyHoverColor:"#C0C6CE"  /*动作按键悬浮态*/

    property color switchKeyNormalColor: "#CED3D9"     /*开关按键默认态*/
    property color switchKeyPressedColor: "#A5ACB5"    /*开关按键按下态*/
    property color switchKeyOpenColor: "#3690F8"   /*开关按键开启态*/
    property color switchKeyOpenPressedColor: "#2C73C6"    /*开关按键开启_按下态*/
    property color switchKeyHoverNormalColor: "#C0C6CE"    /*开关按键默认hover态*/
    property color switchKeyHoverOpenColor: "#328AF0"  /*开关按键按下hover态*/
    property color switchKeyNormalDropShadowColor: "#95A0AD"    /*开关按键默认偏移色*/
    property color switchKeyOpenDropShadowColor: "#1174E5"    /*开关按键开启偏移色*/
    property color switchKeyOpenPressedDropShadowColor: "#165FB3"    /*开关按键开启_按下偏移色*/

    property color currentIMColor: "#328AF0"  /*输入法列表中当前输入法高亮*/
    property color candidateListBackgroundColor: "#DDE0E4"  /*候选词列表背景色*/
    property color candidateDefaultColor: "#262626"  /*候选词默认前景色*/
    property color candidateHighlightColor: "#3790FA"  /*候选词高亮前景色*/
    property color preeditBottomColor: "#E4E6E9" /*预编辑下方分割线颜色*/
    property color dragBarIndicatorColor: "#1D1D1D" /*拖拽条指示器颜色*/

    //圆角相关
    property int virtualKeyboardFloatPlacementRadius: 16
    property int dragBarIndicatorRadius: 2

    //状态相关
    property string letterState: "NORMAL"
    property string symbolState: "NORMAL"
    property string fnSymbolState: "NORMAL"
    property string capslockState: "NORMAL"
    property string shiftState: "NORMAL"
    property string altState: "NORMAL"
    property string ctrlState: "NORMAL"
    property string winState: "NORMAL"
    property string changeIMState: "NORMAL"
    property string switchLayoutButtonState: "NORMAL"
    property string placementMode: isFloatMode ? "FLOAT" : "EXPANSION"

    //可见性相关
    property bool isToolbarVisible: true
    property bool isToolAreaVisible: false
    property bool isKeyBoardLayoutVisible: true
    property bool isAllLayoutListVisible: false
    property string layout: "classic"
    property bool isCurrentIMListVisible: false
    property bool isShiftKeyLongPressed: shiftState === "LONG_PRESSED" ||
                                         shiftState === "OPEN_LONG_PRESSED"

    //内部使用
    signal showToolbar()
    signal showCandidateList()
    signal charKeyClicked()
    signal shiftClicked()
    signal altClicked()
    signal ctrlClicked()
    signal winClicked()

    //后台发送给前台的信号
    signal qmlUpdateCandidateList(var candidateList, int globalCursorIndex)
    signal qmlImDeactivated()

    function processKeyEvent(key, keycode, modifierKeyStates,
                    isRelease, time) {
        model.processKeyEvent(key, keycode, modifierKeyStates, isRelease, time)
    }

    function selectCandidate(index) {
        model.selectCandidate(index)
    }

    function setCurrentIM(imName) {
        model.setCurrentIM(imName)
    }

    function hideVirtualKeyboard() {
        manager.hide()
    }

    function flipPlacementMode() {
        manager.flipPlacementMode()
    }

    function moveBy(offsetX, offsetY) {
        manager.moveBy(offsetX, offsetY)
    }

    function endDrag() {
        manager.endDrag()
    }

    Connections {
        target: virtualKeyboard
        onShowToolbar: {
            isToolbarVisible = true
        }
        onShowCandidateList: {
            isToolbarVisible = false
        }
        onQmlUpdateCandidateList: (candidateList, globalCursorIndex) => {
            if(candidateList.length === 0) {
                showToolbar()
            } else {
                virtualKeyboard.candidateList = candidateList
                virtualKeyboard.globalCursorIndex = globalCursorIndex
                showCandidateList()
            }
        }
    }

    Component.onCompleted: {
        charKeyClicked.connect(onCharKeyClicked)
        shiftClicked.connect(onShiftClicked)
        ctrlClicked.connect(onCtrlClicked)
        altClicked.connect(onAltClicked)
        winClicked.connect(onWinClicked)
    }

    function onCharKeyClicked() {
        updateShiftKeyNormalState()
        ctrlState = "NORMAL"
        altState = "NORMAL"
        winState = "NORMAL"
    }

    function onShiftClicked() {
        ctrlState = "NORMAL"
        altState = "NORMAL"
        winState = "NORMAL"
    }

    function onCtrlClicked() {
        updateShiftKeyNormalState()
        altState = "NORMAL"
        winState = "NORMAL"
    }

    function onAltClicked() {
        updateShiftKeyNormalState()
        ctrlState = "NORMAL"
        winState = "NORMAL"
    }

    function onWinClicked() {
        updateShiftKeyNormalState()
        altState = "NORMAL"
        ctrlState = "NORMAL"
    }

    function updateShiftKeyNormalState() {
        if (!isShiftKeyLongPressed) {
            shiftState = "NORMAL"
        }
    }

    onUniqueNameChanged: {
        for(var i = 0; i<currentIMList.length; i++){
            if(currentIMList[i].includes(uniqueName)){
                currentIM = currentIMList[i]
            }
        }
    }

    Rectangle {
        id: virtualKeyboardContent
        color: virtualKeyboard.virtualKeyboardColor
        radius: virtualKeyboard.radius
        width: view.contentWidth
        height: view.contentHeight

        anchors.horizontalCenter: parent.horizontalCenter
        
        DragBar {
            id: dragBar
            anchors.top: parent.top
        }

        Preedit {
            id: preedit
            anchors.top: dragBar.visible ? dragBar.bottom : parent.top
            cursorPosition: model.preeditCaret
            text: model.preeditText
        }

        Separator {
            id: separator
            anchors.top: preedit.bottom
        }
        
        ToolbarAndCandidateArea {
            id: toolbarAndCandidate
            anchors.top: separator.bottom
        }
        
        KeyboardLayoutArea {
            id: keyboardLayoutArea
            anchors.bottom: virtualKeyboardContent.bottom
        }
        
    }
}
