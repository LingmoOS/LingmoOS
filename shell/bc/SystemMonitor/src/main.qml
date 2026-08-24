import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtGraphicalEffects 1.0
import LingmoUI 1.0 as LingmoUI

LingmoUI.Window {
    id: rootWindow
    visible: true
    width: 900
    height: 600
    minimumWidth: 700
    minimumHeight: 500
    title: qsTr("System Monitor")

    background.opacity: LingmoUI.Theme.darkMode ? 0.7 : 0.5

    property alias stackView: _stackView

    header.height: 40
    contentTopMargin: 0

    LingmoUI.WindowBlur {
        view: rootWindow
        geometry: Qt.rect(rootWindow.x, rootWindow.y, rootWindow.width, rootWindow.height)
        windowRadius: rootWindow.windowRadius
        enabled: true
    }

    LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft
    LayoutMirroring.childrenInherit: true

    RowLayout {
        anchors.fill: parent
        spacing: 0

        SideBar {
            id: sideBar
            Layout.fillHeight: true

            onCurrentIndexChanged: {
                switchPageFromIndex(currentIndex)
            }
        }

        StackView {
            id: _stackView
            Layout.fillWidth: true
            Layout.fillHeight: true
            initialItem: Qt.resolvedUrl(sideBar.model.get(0).page)
            clip: true

            pushEnter: Transition {}
            pushExit: Transition {}
        }
    }

    function addPage(title,name,page,iconSource,category) {
       sideBar.model.append(
        {
            "title": title,
            "name": name,
            "page": page,
            "iconSource": iconSource
        } 
    );
    }

    SystemTray {
        id: systemTray
        onTrayClicked: {
            if (rootWindow.visible)
                rootWindow.hide()
            else
                rootWindow.show()
        }
        onTrayRightClicked: {
            trayMenu.open()
        }
    }

    Menu {
        id: trayMenu
        MenuItem {
            text: qsTr("Show")
            onTriggered: rootWindow.show()
        }
        MenuItem {
            text: qsTr("Hide")
            onTriggered: rootWindow.hide()
        }
        MenuSeparator { }
        MenuItem {
            text: qsTr("Quit")
            onTriggered: Qt.quit()
        }
    }

    Shortcut {
        sequence: "Ctrl+Q"
        onActivated: Qt.quit()
    }

    Shortcut {
        sequence: "Ctrl+W"
        onActivated: rootWindow.hide()
    }

    Shortcut {
        sequence: "Ctrl+R"
        onActivated: systemInfo.refreshProcesses()
    }

    Shortcut {
        sequence: "Ctrl+F"
        onActivated: searchField.forceActiveFocus()
    }

    Component.onCompleted: {
        // Center window
        x = Screen.width / 2 - width / 2
        y = Screen.height / 2 - height / 2
    }

    function switchPageFromIndex(index) {
        _stackView.pop()
        _stackView.push(Qt.resolvedUrl(sideBar.model.get(index).page))
    }

    function switchPageFromName(pageName) {
        for (var i = 0; i < sideBar.model.count; ++i) {
            if (pageName === sideBar.model.get(i).name) {
                switchPageFromIndex(i)
                sideBar.view.currentIndex = i
            }
        }

        // If the window is minimized, it needs to be displayed again.
        rootWindow.show()
        rootWindow.raise()
    }

    // 添加全局数据存储
    property var globalCpuHistory: []
    property var globalMemoryHistory: []
    property var globalNetworkDownloadHistory: []
    property var globalNetworkUploadHistory: []
}