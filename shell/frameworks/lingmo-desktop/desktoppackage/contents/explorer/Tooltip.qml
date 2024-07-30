/*
    SPDX-FileCopyrightText: 2011 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.1
import QtQuick.Layouts 1.0 as Layouts
import org.kde.lingmo.components 3.0 as LingmoComponents
import org.kde.lingmoui 2.20 as LingmoUI

MouseArea {
    id: main

    hoverEnabled: true
    onEntered: toolTipHideTimer.running = false
    onExited: toolTipHideTimer.running = true

    width: LingmoUI.Units.iconSizes.sizeForLabels * 35
    height: LingmoUI.Units.iconSizes.sizeForLabels * 16

    property variant icon
    property string title
    property string description
    property string author
    property string email
    property string license
    property string pluginName
    property bool local

    onClicked: tooltipDialog.visible = false
    Connections {
        target: tooltipDialog
        function onAppletDelegateChanged() {
            if (!tooltipDialog.appletDelegate) {
                return
            }
            icon = tooltipDialog.appletDelegate.icon
            title = tooltipDialog.appletDelegate.title
            description = tooltipDialog.appletDelegate.description
            author = tooltipDialog.appletDelegate.author
            email = tooltipDialog.appletDelegate.email
            license = tooltipDialog.appletDelegate.license
            pluginName = tooltipDialog.appletDelegate.pluginName
            local = tooltipDialog.appletDelegate.local
        }
    }
    LingmoUI.Icon {
        id: tooltipIconWidget
        anchors {
            left: parent.left
            top: parent.top
            margins: 8
        }
        width: LingmoUI.Units.iconSizes.huge
        height: width
        source: main.icon
    }
    Column {
        id: nameColumn
        spacing: 8
        anchors {
            left: tooltipIconWidget.right
            margins: 8
            top: parent.top
            right: parent.right
        }

        LingmoUI.Heading {
            text: title
            level: 2
            anchors.left: parent.left
            anchors.right: parent.right
            height: paintedHeight
            textFormat: Text.PlainText
            wrapMode: Text.Wrap
        }
        LingmoComponents.Label {
            text: description
            textFormat: Text.PlainText
            anchors.left: parent.left
            anchors.right: parent.right
            wrapMode: Text.Wrap
        }
    }
    Layouts.GridLayout {
        columns: 2
        anchors {
            top: (nameColumn.height > tooltipIconWidget.height) ? nameColumn.bottom : tooltipIconWidget.bottom
            topMargin: 16
            horizontalCenter: parent.horizontalCenter
        }
        LingmoComponents.Label {
            text: i18nd("lingmo_shell_org.kde.lingmo.desktop", "License:")
            textFormat: Text.PlainText
            Layouts.Layout.alignment: Qt.AlignVCenter|Qt.AlignRight
        }
        LingmoComponents.Label {
            id: licenseText
            text: license
            textFormat: Text.PlainText
            wrapMode: Text.Wrap
        }
        LingmoComponents.Label {
            text: i18nd("lingmo_shell_org.kde.lingmo.desktop", "Author:")
            textFormat: Text.PlainText
            Layouts.Layout.alignment: Qt.AlignVCenter|Qt.AlignRight
        }
        LingmoComponents.Label {
            text: author
            textFormat: Text.PlainText
            wrapMode: Text.Wrap
        }
        LingmoComponents.Label {
            text: i18nd("lingmo_shell_org.kde.lingmo.desktop", "Email:")
            textFormat: Text.PlainText
            Layouts.Layout.alignment: Qt.AlignVCenter|Qt.AlignRight
        }
        LingmoComponents.Label {
            text: email
            textFormat: Text.PlainText
        }
    }

    LingmoComponents.Button {
        id: uninstallButton
        anchors {
            horizontalCenter: parent.horizontalCenter
            bottom: parent.bottom
        }
        opacity: local ? 1 : 0
        Behavior on opacity {
            NumberAnimation { duration: LingmoUI.Units.longDuration }
        }
        iconSource: "application-exit"
        text: i18nd("lingmo_shell_org.kde.lingmo.desktop", "Uninstall")
        onClicked: {
            widgetExplorer.uninstall(pluginName)
            tooltipDialog.visible = false
        }
    }
}
