/*
 *   Copyright 2021 Reion Wong <reionwong@gmail.com>
 *   Copyright 2018 Camilo Higuita <milo.h@aol.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it  be useful,
 *   but WITHOUT ANY WARRANTY;  even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.3
import QtQuick.Window 2.12

import LingmoUI 1.0 as LingmoUI
import Lingmo.TermWidget 1.0

Page {
    id: control

    height: _tabView.height
    width: _tabView.width
    focus: true

    background: Rectangle {
        color: "transparent"
    }

    signal urlsDropped(var urls)
    signal keyPressed(var event)
    signal terminalClosed()

    property string path: "$PWD"
    property alias terminal: _terminal
    readonly property QMLTermSession session: _session

    title: _session.title

    Component.onCompleted: {
        //settings.colorschemes = _terminal.availableColorSchemes
    }

    onUrlsDropped: {
        for (var i in urls)
            _session.sendText(urls[i].replace("file://", "") + " ")
    }

    onKeyPressed: {
        if ((event.key === Qt.Key_A)
                && (event.modifiers & Qt.ControlModifier)
                && (event.modifiers & Qt.ShiftModifier)) {
            _terminal.selectAll()
            event.accepted = true
        }

        if ((event.key === Qt.Key_C)
                && (event.modifiers & Qt.ControlModifier)
                && (event.modifiers & Qt.ShiftModifier)) {
            _copyAction.trigger()
            event.accepted = true
        }

        if ((event.key === Qt.Key_V)
                && (event.modifiers & Qt.ControlModifier)
                && (event.modifiers & Qt.ShiftModifier)) {
            _pasteAction.trigger()
            event.accepted = true
        }

        if ((event.key === Qt.Key_Q)
                && (event.modifiers & Qt.ControlModifier)
                && (event.modifiers & Qt.ShiftModifier)) {
            Qt.quit()
        }

        if ((event.key === Qt.Key_T)
                && (event.modifiers & Qt.ControlModifier)
                && (event.modifiers & Qt.ShiftModifier)) {
            root.openNewTab()
            event.accepted = true
        }

        if ((event.key === Qt.Key_W)
                && (event.modifiers & Qt.ControlModifier)
                && (event.modifiers & Qt.ShiftModifier)) {
            root.closeCurrentTab()
            event.accepted = true
        }

        if (event.key === Qt.Key_Tab && event.modifiers & Qt.ControlModifier) {
            root.toggleTab()
            event.accepted = true
        }

        if (event.key === Qt.Key_F11) {
            root.visibility = root.isFullScreen ? Window.Windowed : Window.FullScreen
            event.accepted = true
        }
    }

    QMLTermWidget {
        id: _terminal
        anchors.fill: parent
        colorScheme: LingmoUI.Theme.darkMode ? settings.darkcolorScheme : settings.lightcolorScheme
        font.family: settings.fontName
        font.pointSize: settings.fontPointSize
        blinkingCursor: settings.blinkingCursor
        wordCharacters: settings.wordCharacters
        fullCursorHeight: true
        backgroundOpacity: 0.4
        keyboardCursorShape: settings.keyboardCursorShape

        Keys.enabled: true
        Keys.onPressed: control.keyPressed(event)

        onBackgroundColorChanged: {
            if(LingmoUI.Theme.darkMode)
                {
                    settings.darkbackgroundColor = backgroundColor
                }else
                {
                    settings.lightbackgroundColor = backgroundColor
                }
            }

        session: QMLTermSession {
            id: _session
            onFinished: control.terminalClosed()
            initialWorkingDirectory: control.path
        }

        MouseArea {
            anchors.fill: parent
            propagateComposedEvents: true
            cursorShape: _terminal.terminalUsesMouse ? Qt.ArrowCursor : Qt.IBeamCursor
            acceptedButtons:  Qt.RightButton | Qt.LeftButton

            onDoubleClicked: {
                 var coord = correctDistortion(mouse.x, mouse.y)
                 _terminal.simulateMouseDoubleClick(coord.x, coord.y, mouse.button, mouse.buttons, mouse.modifiers)
            }

            onPressed: {
                if ((!_terminal.terminalUsesMouse || mouse.modifiers & Qt.ShiftModifier)
                        && mouse.button == Qt.RightButton) {
                    updateMenu()
                    terminalMenu.popup()
                } else {
                    var coord = correctDistortion(mouse.x, mouse.y)
                    _terminal.simulateMousePress(coord.x, coord.y, mouse.button, mouse.buttons, mouse.modifiers)
                }
            }

            onReleased: {
                var coord = correctDistortion(mouse.x, mouse.y)
                _terminal.simulateMouseRelease(coord.x, coord.y, mouse.button, mouse.buttons, mouse.modifiers)
            }

            onPositionChanged: {
                var coord = correctDistortion(mouse.x, mouse.y)
                _terminal.simulateMouseMove(coord.x, coord.y, mouse.button, mouse.buttons, mouse.modifiers)
            }

            onClicked: {
                if (mouse.button === Qt.RightButton) {
                    updateMenu()
                    terminalMenu.popup()
                } else if(mouse.button === Qt.LeftButton) {
                    _terminal.forceActiveFocus()
                }

                // control.clicked()
            }
        }

        Component.onCompleted: {
            _session.startShellProgram()
            _terminal.forceActiveFocus()
        }
    }

    Action {
        id: _copyAction
        text: qsTr("Copy")
        onTriggered: _terminal.copyClipboard()
    }

    Action {
        id: _pasteAction
        text: qsTr("Paste")
        onTriggered: _terminal.pasteClipboard()
    }

    Menu {
        id: terminalMenu
        spacing: 0
        MenuItem {
            id: copyMenuItem
            action: _copyAction
        }

        MenuItem {
            id: pasteMenuItem
            text: qsTr("Paste")
            action: _pasteAction
        }

        MenuItem {
            id: selectAllItem
            text: qsTr("Select All")
            onTriggered: _terminal.selectAll()
        }

        MenuSeparator{}

        MenuItem {
            id: clearScreenItem
            text: qsTr("Clear Screen")
            onTriggered: _session.sendText("clear\n")
        }

        Menu {
            id: bookmarkMenu
            title: qsTr("Bookmark")
            Repeater {
                model: settings.bookmark
                delegate: MenuItem {
                text: settings.bookmark[index]

                onTriggered:{
                    _session.sendText("cd "+settings.bookmark[index]+"\n")
                    }
                }
            }
        }

        MenuItem {
            text: qsTr("Add to bookmark")
            onTriggered: {
                var mbm = settings.bookmark;
                mbm.unshift(_session.currentDir)
                settings.bookmark = mbm
            }
        }

        MenuSeparator{}

        Menu {
            id: searchMenu
            title: qsTr("Search")
            Repeater {
                model: settings.searchUrl
                delegate: MenuItem {
                text: settings.searchUrl[index][0]

                onTriggered:{
                    var url = settings.searchUrl[index][1];
                    url = url.replace("{KeyWord}",_terminal.getSelectedText)
                    Qt.openUrlExternally(url)
                    }
                }
            }
        }

        MenuItem {
            text: qsTr("Open File Manager")
            onTriggered: Process.openFileManager(_session.currentDir)
        }

        MenuItem {
            text: root.isFullScreen ? qsTr("Exit full screen") : qsTr("Full screen")
            onTriggered: {
                root.visibility = root.isFullScreen ? Window.Windowed : Window.FullScreen
            }
        }

        MenuSeparator{}

        MenuItem {
            text: qsTr("Settings")
            onTriggered: {
                settingsDialog.show()
                settingsDialog.raise()
            }
        }
    }

    ScrollBar {
        id: _scrollbar
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.bottomMargin: LingmoUI.Units.smallSpacing * 1.5
        hoverEnabled: true
        active: hovered || pressed
        orientation: Qt.Vertical
        size: (_terminal.lines / (_terminal.lines + _terminal.scrollbarMaximum - _terminal.scrollbarMinimum))
        position: _terminal.scrollbarCurrentValue / (_terminal.lines + _terminal.scrollbarMaximum)
        onPositionChanged: {
            _terminal.scrollbarCurrentValue = position * (_terminal.lines + _terminal.scrollbarMaximum)
        }
    }

    DropArea {
        id: _dropArea
        anchors.fill: parent
        onDropped: {
            if (drop.hasUrls) {
                control.urlsDropped(drop.urls)
            } else if (drop.hasText) {
                _session.sendText(drop.text)
            }
        }
    }

    function forceActiveFocus() {
        _terminal.forceActiveFocus()
    }

    function correctDistortion(x, y) {
        x = x / width
        y = y / height

        var cc = Qt.size(0.5 - x, 0.5 - y)
        var distortion = 0

        return Qt.point((x - cc.width  * (1 + distortion) * distortion) * _terminal.width,
                        (y - cc.height * (1 + distortion) * distortion) * _terminal.height)
    }

    function updateMenu() {
        if(_terminal.selectedText)
        {
            copyMenuItem.height = copyMenuItem.implicitHeight
        }else{
            copyMenuItem.height = 0
        }

        if(Utils.text())
        {
            pasteMenuItem.height = pasteMenuItem.implicitHeight
        }else{
            pasteMenuItem.height = 0
        }
        copyMenuItem.visible = _terminal.selectedText
        pasteMenuItem.visible = Utils.text()
    }
}
