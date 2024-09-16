// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.0
import QtQuick.Window 2.11
import QtQuick.Layouts 1.11
import org.deepin.dtk 1.0

TitleBar {
    id: titleBar
    icon.name: "music"
    title: "title custom"

    property string appProductName: Qt.application.displayName
    property string appLicense: "LGPL-3.0-or-later"

//    menu: Menu {
//        delegate: MenuItem {
//            icon.name: "emblem-checked"
//            icon.color: palette.highlight
//        }
//        ThemeMenu {}

//        MenuSeparator {
//            contentItem: Rectangle {
//                implicitHeight: 1
//                color: "black"
//            }
//        }
//        HelpAction {}
//        AboutAction {}
//        QuitAction {}

//        Action {
//            text: qsTr("custom action")
//        }
//        MenuItem {
//            text: qsTr("Window State")
//            onTriggered: toggleWindowState()
//        }
//    }
    aboutDialog: AboutDialog {
        productIcon: "music"
        modality: Qt.NonModal
        productName: qsTr(appProductName)
        companyLogo: "file://" + DTK.deepinDistributionOrgLogo
        websiteName: DTK.deepinWebsiteName
        websiteLink: DTK.deepinWebsiteLink
        description: "qml inspect is used to developer as a debug tool."
        license: appLicense === "" ? "" : qsTr(String("%1 is released under %2").arg(appProductName).arg(appLicense))
    }

    embedMode: false
//    separatorVisible: false
    autoHideOnFullscreen: true
//    menuDisabled: true

//    content: RowLayout {
//        LineEdit {
//            text: "label2"
//        }
//        LineEdit {
//            text: "center"
//            Layout.alignment: Qt.AlignCenter
//        }
//        Button {
//            text: "center custom"
//            Layout.alignment: Qt.AlignRight
//        }
//    }

    focus: true
    Keys.onSpacePressed: Window.window.visibility = Window.FullScreen
}
