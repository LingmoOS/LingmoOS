/**
 * New LingmoOS terminal
 * Copyright 2023 LingmoOS Team
 */
import QtQuick 2.12
import QtQml.Models 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtGraphicalEffects 1.0

import Lingmo.TermWidget 1.0
import LingmoUI 1.0 as LingmoUI

LingmoUI.Window {
    id: root
    minimumWidth: 400
    minimumHeight: 300
    width: settings.width
    height: settings.height
    title: currentItem && currentItem.terminal ? currentItem.terminal.session.title : ""

    GlobalSettings { id: settings }

    background.color: LingmoUI.Theme.darkMode ? settings.darkbackgroundColor : settings.lightbackgroundColor
    background.opacity: root.compositing ? settings.opacity : 1
//    background.opacity: rootWindow.compositing ? 0.95 : 1.0
    header.height: 40

    property int currentIndex: -1
    property alias currentItem: _tabView.currentItem
    readonly property QMLTermWidget currentTerminal: currentItem ? currentItem.terminal : null

    // MenuBar {
    //     id: menuBar

    //     Menu {
    //         id: helpMenu
    //         title: qsTr("Help")

    //         MenuItem {
    //             text: qsTr("About Terminal")
    //             onTriggered: aboutDialog.show()
    //         }
    //     }
    // }

    // LingmoUI.AboutDialog {
    //     id: aboutDialog
    //     name: root.title
    //     description: qsTr("LingmoOS terminal simulator")
    //     iconSource: "image://icontheme/lingmo-terminal"
    // }

    ExitPromptDialog {
        id: exitPrompt

        property var index: -1

        onOkBtnClicked: {
            if (index != -1) {
                closeTab(index)
            } else {
                onOkBtnClicked: Qt.quit()
            }
        }
    }

    SettingsDialog {
        id: settingsDialog
    }

    LingmoUI.WindowBlur {
        view: root
        geometry: Qt.rect(root.x, root.y, root.width, root.height)
        windowRadius: root.background.radius
        enabled: settings.blur
    }

    onClosing: {
        if (!root.isMaximized && !root.isFullScreen) {
            settings.width = root.width
            settings.height = root.height
        }

        // Exit prompt.
        for (var i = 0; i < _tabView.contentModel.count; ++i) {
            var obj = _tabView.contentModel.get(i)
            if (obj.session.hasActiveProcess) {
                exitPrompt.index = -1
                exitPrompt.visible = true
                close.accepted = false
                break
            }
        }
    }

    headerItem: Item {
        LingmoUI.TabBar {
            id: _tabbar
            anchors.fill: parent
            anchors.margins: LingmoUI.Units.smallSpacing / 2
            anchors.rightMargin: LingmoUI.Units.largeSpacing * 4

            currentIndex: _tabView.currentIndex
            model: _tabView.count

            onNewTabClicked: openNewTab()

            delegate: LingmoUI.TabButton {
                id: _tabBtn
                text: _tabView.contentModel.get(index).title
                Layout.fillHeight: true
                height: _tabbar.height - LingmoUI.Units.smallSpacing / 2
                width: Math.min(_tabbar.width / _tabbar.count,
                                _tabBtn.contentWidth)

                ToolTip.delay: 500
                ToolTip.timeout: 5000

                checked: _tabView.currentIndex === index

                font.pointSize: 9
                font.family: "Noto Sans Mono"

                ToolTip.visible: hovered
                ToolTip.text: _tabView.contentModel.get(index).title

                onClicked: {
                    _tabView.currentIndex = index
                    _tabView.currentItem.forceActiveFocus()
                }

                onCloseClicked: {
                    root.closeProtection(index)
                }
            }

//            Repeater {
//                id: _repeater
//                model: _tabView.count

//                LingmoUI.TabButton {
//                    id: _tabBtn
//                    text: _tabView.contentModel.get(index).title
//                    implicitHeight: parent.height
////                    implicitWidth: _repeater.count === 1 ? 200
////                                                         : parent.width / _repeater.count

//                    implicitWidth: Math.min(_tabBtn.contentWidth,
//                                            parent.width / _repeater.count)

//                    ToolTip.delay: 1000
//                    ToolTip.timeout: 5000

//                    checked: _tabView.currentIndex === index

//                    font.pointSize: 9
//                    font.family: "Noto Sans Mono"

//                    ToolTip.visible: hovered
//                    ToolTip.text: _tabView.contentModel.get(index).title

//                    onClicked: {
//                        _tabView.currentIndex = index
//                        _tabView.currentItem.forceActiveFocus()
//                    }

//                    onCloseClicked: {
//                        root.closeProtection(index)
//                    }
//                }
//            }
        }
    }

    ColumnLayout {
        anchors.fill: parent

        LingmoUI.TabView {
            id: _tabView
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
    }

    Component.onCompleted: {
        openTab("$PWD")
    }

    function openNewTab() {
        if (_tabView.currentItem) {
            openTab(_tabView.currentItem.session.currentDir)
        } else {
            openTab("$HOME")
        }
    }

    function openTab(path) {
        if (_tabView.contentModel.count > 7)
            return

        const component = Qt.createComponent("Terminal.qml");
        if (component.status === Component.Ready) {
            const index = _tabView.contentModel.count
            const object = _tabView.addTab(component, {path: path})
            object.terminalClosed.connect(() => closeTab(index))
        }
    }

    function closeProtection(index) {
        var obj = _tabView.contentModel.get(index)
        if (obj.session.hasActiveProcess) {
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
    }

    function closeCurrentTab() {
        closeProtection(_tabView.currentIndex)
    }

    function toggleTab() {
        var nextIndex = _tabView.currentIndex
        ++nextIndex
        if (nextIndex > _tabView.contentModel.count - 1)
            nextIndex = 0

        _tabView.currentIndex = nextIndex
    }
}
