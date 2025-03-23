// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.11
import org.deepin.dtk 1.0 as D
import org.deepin.dtk.style 1.0 as DS
import org.deepin.dtk.settings 1.0 as Settings
import org.deepin.dtk 1.0

DialogWindow {
    id: control

    property list<Settings.SettingsGroup> groups
    property D.Config config
    property Settings.SettingsContainer container : Settings.SettingsContainer {
        id: settingsContainer
        config: control.config
        navigationTitle: Settings.NavigationTitle {
            width: navigationView.width - navigationView.leftMargin - navigationView.rightMargin
            checked: navigationView.currentIndex === Settings.SettingsGroup.index
            onClicked: navigationView.currentIndex = Settings.SettingsGroup.index
        }
        contentTitle: Settings.ContentTitle {}
        contentBackground: Settings.ContentBackground {}
        groups: control.groups
    }
    property alias navigationView: navigationView
    property alias contentView: contentView

    ScrollView {
        id: navigationBg
        width: DS.Style.settings.navigation.width
        background: Rectangle {
            anchors.fill: parent
            color: palette.base
        }

        ListView {
            id: navigationView
            model: container.navigationModel
            leftMargin: DS.Style.settings.navigation.margin
            rightMargin: DS.Style.settings.navigation.margin
            currentIndex: 0
            onCurrentIndexChanged: {
                contentView.currentIndex = currentIndex
            }
        }
    }

    ScrollView {
        id: contentBg
        anchors {
            right: parent.right
            left: navigationBg.right
            top: control.top
        }
        padding: DS.Style.settings.content.margin
        background: Rectangle {
            anchors.fill: parent
            color: palette.base
        }

        ListView {
            id: contentView
            model: container.contentModel
            highlightRangeMode: ListView.StrictlyEnforceRange
            currentIndex: 0
            onCurrentIndexChanged: {
                navigationView.currentIndex = currentIndex
            }
            footer: Item {
                width: parent.width
                height: DS.Style.settings.content.resetButtonHeight

                Button {
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.bottom: parent.bottom
                    text: qsTr("Restore Defaults")

                    onClicked: {
                        settingsContainer.resetSettings();
                    }
                }
            }
        }
    }
}
