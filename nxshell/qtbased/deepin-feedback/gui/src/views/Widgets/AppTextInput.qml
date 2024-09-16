/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

import QtQuick 2.1
import Deepin.Widgets 1.0

FocusScope{
    id: root
    width: 200
    height: 40
    state: "normal"

    property alias input: textInput
    property alias text: textInput.text
    property alias tip: tipText.text
    property alias backgroundColor: rootRec.color
    property int fontPixeSize: 12
    property int maxStrLength: 0
    property bool canChangeState: true
    property bool inWarningState: false

    signal textChange(string text)
    signal keyPressed(var event)

    //Use Unicode Length
    function getStrLeng(str) {
        var myLen = 0;

        for (var i = 0; i < str.length; i++) {
            if (str.charCodeAt(i) > 0 && str.charCodeAt(i) < 128)
                myLen++;
            else
                myLen += 2;
        }
        return myLen;
    }

    function isContentOutOfRange(){
        return maxStrLength > 0 && getStrLeng(textInput.text) > maxStrLength
    }

    onInWarningStateChanged: {
        if (inWarningState)
            state = "warning"
        else{
            if (focus)
                state = "actived"
            else
                state = "normal"
        }
    }

    onFocusChanged: {
        if (canChangeState && !inWarningState){
            if (focus)
                state = "actived"
            else
                state = "normal"
        }
    }

    Rectangle {
        id: rootRec
        radius: 3
        anchors.fill: parent
        color: bgNormalColor
        clip: true

        TextInput {
            id: textInput

            focus: true
            color: root.enabled ? textNormalColor : "#bebebe"
            selectionColor: "#61B5F8"
            selectByMouse: true
            verticalAlignment: TextInput.AlignVCenter
            font.pixelSize: fontPixeSize
            clip: true

            width: parent.width - 10
            height: parent.height
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            anchors.leftMargin: 5

            onTextChanged: {
                if (maxStrLength > 0 && getStrLeng(textInput.text) >= maxStrLength){
                    textInput.maximumLength = textInput.length
                    inWarningState = true
                }
                else
                    inWarningState = false

                root.textChange(text)
            }
            Keys.onPressed: {
                root.keyPressed(event)
            }
        }

        Text {
            id: tipText
            anchors.fill: parent
            anchors.leftMargin: 5
            color: "#bebebe"
            verticalAlignment: Text.AlignVCenter
            font.pixelSize: fontPixeSize
            clip: true

            opacity: textInput.text == "" && root.state == "normal" ? 1 : 0

            Behavior on opacity {
                NumberAnimation {duration: 150}
            }
        }
    }


    states: [
        State {
            name: "normal"
            PropertyChanges {target: rootRec; border.color: buttonBorderColor}
        },
        State {
            name: "actived"
            PropertyChanges {target: rootRec; border.color: buttonBorderActiveColor}
        },
        State {
            name: "warning"
            PropertyChanges {target: rootRec; border.color: buttonBorderWarningColor}
        }
    ]

}
