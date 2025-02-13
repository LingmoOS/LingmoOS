/*
 * Copyright (C) 2023 LingmoOS Team.
 */

import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Dialogs 1.3
import LingmoUI 1.0 as LingmoUI
import Lingmo.TextEditor 1.0

LingmoUI.Window {
    id: root
    width: 640
    height: 480
    minimumWidth: 300
    minimumHeight: 300
    visible: true
    title: qsTr("Lingmo OS Text Editor")

    FileHelper {
        id: fileHelper

        onNewPath: {
            _tabView.addTab(textEditorComponent, { fileUrl: "file://" + path, newFile: false })
        }

        onUnavailable: {
            root.showPassiveNotification(qsTr("%1 doesn't exists").arg(path), 3000)
        }
    }

    ExitPromptDialog {
        id: exitPrompt

        property var index: -1

        onOkBtnClicked: {
            if (index != -1)
                closeTab(index)
            else
                Qt.quit()
        }
    }

    headerItem: Item {
        Rectangle {
            anchors.fill: parent
            color: LingmoUI.Theme.backgroundColor
        }

        LingmoUI.TabBar {
            id: _tabbar
            anchors.fill: parent
            anchors.margins: LingmoUI.Units.smallSpacing / 2
            anchors.rightMargin: LingmoUI.Units.largeSpacing * 4

            model: _tabView.count
            currentIndex : _tabView.currentIndex

            onNewTabClicked: {
                addTab()
            }

            delegate: LingmoUI.TabButton {
                id: _tabBtn
                text: _tabView.contentModel.get(index).tabName
                implicitHeight: _tabbar.height
                implicitWidth: Math.min(_tabbar.width / _tabbar.count,
                                        _tabBtn.contentWidth)

                ToolTip.delay: 1000
                ToolTip.timeout: 5000

                checked: _tabView.currentIndex === index

                ToolTip.visible: hovered
                ToolTip.text: _tabView.contentModel.get(index).fileUrl

                onClicked: {
                    _tabView.currentIndex = index
                    _tabView.currentItem.forceActiveFocus()
                }

                onCloseClicked: {
                    closeProtection(index)
                }
            }
        }
    }

    DropArea {
        id: _dropArea
        anchors.fill: parent

        onDropped: {
            if (drop.hasUrls) {
                for (var i = 0; i < drop.urls.length; ++i) {
                    root.addPath(drop.urls[i])
                }
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        LingmoUI.TabView {
            id: _tabView
            Layout.fillWidth: true
            Layout.fillHeight: true
        }

        Item {
            id: _bottomItem
            z: 999
            Layout.fillWidth: true
            Layout.preferredHeight: 20 + LingmoUI.Units.smallSpacing

            Rectangle {
                anchors.fill: parent
                color: LingmoUI.Theme.backgroundColor
            }

            ColumnLayout {
                anchors.fill: parent
                anchors.leftMargin: LingmoUI.Units.smallSpacing
                anchors.rightMargin: LingmoUI.Units.smallSpacing
                anchors.bottomMargin: LingmoUI.Units.smallSpacing

                Label {
                    text: _tabView.currentItem ? qsTr("Characters %1").arg(_tabView.currentItem.characterCount)
                                               : ""
                }
            }
        }
    }

    function addPath(path) {
        fileHelper.addPath(path)
        // _tabView.addTab(textEditorComponent, { fileUrl: path, newFile: false })
    }

    function addTab() {
        _tabView.addTab(textEditorComponent, { fileUrl: "", newFile: true, fileName: qsTr("Untitled") })
        _tabView.currentItem.forceActiveFocus()
    }

    FileDialog {
        id: fileOpenDialog
        title: qsTr("Open...")
        folder: shortcuts.home
        nameFilters: [ qsTr("All files (*)") ]

        selectExisting: true
        selectFolder: false
        selectMultiple: true

        onAccepted: {
            for (var i = 0; i < fileOpenDialog.fileUrls.length; i++)
                addPath(fileOpenDialog.fileUrls[i].toString().substr(7))
        }
        Component.onCompleted: visible = false
    }

    function open() {
        fileOpenDialog.open()
    }

    function closeAll() {
        for (var i = 0; i < _tabView.contentModel.count; i++) {
            var obj = _tabView.contentModel.get(i)
            if (obj.documentModified) {
                exitPrompt.index = -1
                exitPrompt.visible = true
                return false
            }
        }
        return true
    }

    onClosing: {
        close.accepted = closeAll()
    }

    function closeProtection(index) {
        var obj = _tabView.contentModel.get(index)
        if (obj.documentModified) {
            exitPrompt.index = index
            exitPrompt.visible = true
            return
        }

        closeTab(index)
    }

    function closeTab(index) {
        _tabView.closeTab(index)

        if (_tabView.contentModel.count === 0)
            Qt.quit()

        _tabView.currentItem.forceActiveFocus()
    }

    function closeCurrentTab() {
        closeProtection(_tabView.currentIndex)
    }

    function toggleTab(arg) { //arg = -1 (forward) or 1 (backward)
        var nextIndex = _tabView.currentIndex + arg
        if (nextIndex > _tabView.contentModel.count - 1)
            nextIndex = 0
        if (nextIndex < 0)
            nextIndex = _tabView.contentModel.count - 1

        _tabView.currentIndex = nextIndex
        _tabView.currentItem.forceActiveFocus()
    }

    Component {
        id: textEditorComponent

        TextEditor {
            fileUrl: ""
            newFile: true
        }
    }

    Component.onCompleted: {
    }
}
