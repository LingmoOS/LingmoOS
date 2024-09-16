/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/
import QtQuick 2.1
import QtQuick.Window 2.1
import QtQuick.Controls 1.0
import Deepin.Widgets 1.0

Item {
    id: combobox
    height: 40

    property bool hovered: false
    property bool pressed: false
    property bool settingText: false

    property alias text: currentLabel.text
    property alias menu: menu

    property var parentWindow
    property var labels:projectNameList
    property int selectIndex: -1

    property string _searchMd5: ""

    signal clicked
    signal searchResultCountChanged(int count)
    signal menuSelect(int index)

    onSelectIndexChanged: menu.currentIndex = selectIndex
    Component.onCompleted: {
        if(selectIndex != -1){
            text = menu.labels[selectIndex]
            menu.currentIndex = selectIndex
        }

        _searchMd5 = dbusSearch.NewSearchWithStrList(projectNameList.concat(projectEnNameList))[0]
    }

    onClicked: {
        currentLabel.forceActiveFocus()
        showMenu()
    }


    function showMenu() {
        var pos = mapToItem(null, 0, 0)
        var x = parentWindow.x + pos.x
        var y = parentWindow.y + pos.y + height
        var w = width

        menu.x = x - menu.frameEdge + 1
        menu.y = y - menu.frameEdge
        menu.width = w + menu.frameEdge * 2 -2
        menu.visible = true
    }

    function hideMenu(){
        menu.visible = false
        combobox.labels = projectNameList
    }

    function setText(value){
        settingText = true
        currentLabel.text = value
        settingText = false
    }

    function setTextInputState(value) {
        currentLabel.state = value
    }

    Item {
        id:background
        height: parent.height
        anchors.fill: parent

        AppTextInput {
            id:currentLabel
            state: "normal"
            width: combobox.width
            height: parent.height
            anchors.left: parent.left
            tip: dsTr("Please select feedback project firstly ")

            onTextChange: {
                if (settingText)
                    return

                if(text == ""){
                    combobox.labels = projectNameList
                    mainWindow.disableTextInput()
                    mainWindow.saveDraft()
                    mainWindow.lastTarget = ""
                    mainWindow.clearDraft()
                }
                else{
                    var inEnLang = dsslocale.lang.indexOf("en_") != -1
                    var searchResult =searchResult = dbusSearch.SearchString(text, _searchMd5)
                    var appList = new Array()

                    for(var i in searchResult) {
                        var v = inEnLang ? searchResult[i] : dsTr(searchResult[i]);
                        if (appList.indexOf(v) != -1)
                            break;
                        appList.push(v)
                    }

                    combobox.searchResultCountChanged(appList.length)
                    combobox.labels = appList

                    if (!menu.visible && appList.length > 0) {
                        showMenu()
                    }
                    else if(appList.length <= 0) {
                        hideMenu()
                    }
                }
            }
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    currentLabel.forceActiveFocus()
                    showMenu()
                }
            }
            Keys.onPressed: {
                switch (event.key )
                {
                case Qt.Key_Escape:
                    hideMenu()
                    break
                case Qt.Key_Enter:
                case Qt.Key_Return:
                    menu.selectMenu()
                    break
                case Qt.Key_Up:
                    menu.menuUp()
                    break
                case Qt.Key_Down:
                    menu.menuDown()
                    break
                default:
                    break
                }
            }

            Image {
                anchors.right: parent.right

                source: "qrc:///views/Widgets/images/arrow_down_normal.png"

                MouseArea{
                    anchors.fill: parent
                    hoverEnabled: true

                    onEntered: {
                        combobox.hovered = true
                        parent.source = "qrc:///views/Widgets/images/arrow_down_hover.png"
                    }

                    onExited: {
                        combobox.hovered = false
                        parent.source = "qrc:///views/Widgets/images/arrow_down_normal.png"
                    }

                    onPressed: {
                        combobox.pressed = true
                        parent.source = "qrc:///views/Widgets/images/arrow_down_press.png"
                    }
                    onReleased: {
                        combobox.pressed = false
                        combobox.hovered = containsMouse
                        parent.source = "qrc:///views/Widgets/images/arrow_down_normal.png"
                    }

                    onClicked: {
                        combobox.clicked()
                    }
                }

                Rectangle {
                    anchors.left: parent.left
                    width: 1
                    height: parent.height
                    visible: currentLabel.focus
                    color: buttonBorderActiveColor
                }
            }
        }
    }

    AppComboBoxMenu {
        id: menu
        parentWindow: combobox.parentWindow
        labels: combobox.labels
        onMenuSelect: {
            var truelly_index = projectNameList.indexOf(menu.labels[index])
            combobox.menuSelect(truelly_index)
            selectIndex = index
            combobox.text = menu.labels[selectIndex]

            hideMenu()
        }
    }

}
