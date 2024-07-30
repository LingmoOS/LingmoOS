// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: %{CURRENT_YEAR} %{AUTHOR} <%{EMAIL}>

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.lingmoui as LingmoUI
import org.kde.%{APPNAMELC}

LingmoUI.ApplicationWindow {
    id: root

    title: i18n("%{APPNAME}")

    minimumWidth: LingmoUI.Units.gridUnit * 20
    minimumHeight: LingmoUI.Units.gridUnit * 20

    onClosing: App.saveWindowGeometry(root)

    onWidthChanged: saveWindowGeometryTimer.restart()
    onHeightChanged: saveWindowGeometryTimer.restart()
    onXChanged: saveWindowGeometryTimer.restart()
    onYChanged: saveWindowGeometryTimer.restart()

    Component.onCompleted: App.restoreWindowGeometry(root)

    // This timer allows to batch update the window size change to reduce
    // the io load and also work around the fact that x/y/width/height are
    // changed when loading the page and overwrite the saved geometry from
    // the previous session.
    Timer {
        id: saveWindowGeometryTimer
        interval: 1000
        onTriggered: App.saveWindowGeometry(root)
    }

    property int counter: 0

    globalDrawer: LingmoUI.GlobalDrawer {
        isMenu: !LingmoUI.Settings.isMobile
        actions: [
            LingmoUI.Action {
                text: i18n("Plus One")
                icon.name: "list-add"
                onTriggered: root.counter += 1
            },
            LingmoUI.Action {
                text: i18n("About %{APPNAME}")
                icon.name: "help-about"
                onTriggered: root.pageStack.pushDialogLayer("qrc:About.qml")
            },
            LingmoUI.Action {
                text: i18n("Quit")
                icon.name: "application-exit"
                onTriggered: Qt.quit()
            }
        ]
    }

    contextDrawer: LingmoUI.ContextDrawer {
        id: contextDrawer
    }

    pageStack.initialPage: page

    LingmoUI.Page {
        id: page

        title: i18n("Main Page")

        actions: [
            LingmoUI.Action {
                text: i18n("Plus One")
                icon.name: "list-add"
                tooltip: i18n("Add one to the counter")
                onTriggered: root.counter += 1
            }
        ]

        ColumnLayout {
            width: page.width

            anchors.centerIn: parent

            LingmoUI.Heading {
                Layout.alignment: Qt.AlignCenter
                text: root.counter === 0 ? i18n("Hello, World!") : root.counter
            }

            QQC2.Button {
                Layout.alignment: Qt.AlignHCenter
                text: i18n("+ 1")
                onClicked: root.counter += 1
            }
        }
    }
}
