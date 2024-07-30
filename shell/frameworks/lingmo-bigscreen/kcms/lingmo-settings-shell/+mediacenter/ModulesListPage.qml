/*

    SPDX-FileCopyrightText: 2011-2014 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.14
import QtQuick.Layouts 1.14
import QtQuick.Controls 2.14 as Controls
import org.kde.lingmoui 2.12 as LingmoUI
import org.kde.lingmo.core 2.0 as LingmoCore
import org.kde.lingmo.settings 0.1

LingmoUI.Page {
    id: settingsRoot

    property alias currentIndex: listView.currentIndex

    LingmoUI.Theme.colorSet: LingmoUI.Theme.View
    background: Rectangle {
        color: LingmoUI.Theme.backgroundColor
    }

    Component {
        id: settingsModuleDelegate
        Controls.ItemDelegate {
            id: delegateItem

            height: listView.height
            width: settingsRoot.width > LingmoUI.Units.gridUnit * 20 ? settingsRoot.width/4 : settingsRoot.width
            enabled: true
            checked: listView.currentIndex == index
            leftPadding: LingmoUI.Units.largeSpacing
            background: null
            Keys.onReturnPressed: clicked()
            contentItem: Item {
                ColumnLayout {
                    anchors.centerIn: parent
                    spacing: LingmoUI.Units.largeSpacing
                    LingmoUI.Icon {
                        id: iconItem
                        Layout.alignment: Qt.AlignCenter
                        selected: delegateItem.down
                        Layout.maximumWidth: Layout.preferredWidth
                        Layout.preferredWidth: listView.currentIndex == index ? LingmoCore.Units.iconSizes.enormous : LingmoCore.Units.iconSizes.huge
                        Layout.preferredHeight: Layout.preferredWidth
                        Behavior on Layout.preferredWidth {
                            NumberAnimation {
                                duration: LingmoUI.Units.longDuration
                                easing.type: Easing.InOutQuad
                            }
                        }
                        source: iconName
                    }

                    Controls.Label {
                        Layout.fillWidth: true
                        text: name
                        horizontalAlignment: Text.AlignHCenter
                    }
                    Controls.Label {
                        text: description
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignHCenter
                        font.pointSize: LingmoUI.Theme.defaultFont.pointSize -1
                        opacity: 0.6
                        elide: Text.ElideRight
                    }
                }
            }

            onClicked: {
                print("Clicked index: " + index + " current: " + listView.currentIndex + " " + name + " curr: " + rootItem.currentModule);
                // Only the first main page has a kcm property
                var container = kcmContainer.createObject(pageStack, {"kcm": model.kcm, "internalPage": model.kcm.mainUi});
                pageStack.push(container);
            }
        }
    }

    // This is pretty much a placeholder of what will be the sandboxing mechanism: this element will be a wayland compositor that will contain off-process kcm pages
    Component {
        id: kcmContainer

        KCMContainer {}
    }

    contentItem: ListView {
        id: listView
        focus: true
        spacing: 0
        orientation: ListView.Horizontal
        activeFocusOnTab: true
        keyNavigationEnabled: true
        highlightFollowsCurrentItem: true
        highlightMoveDuration: LingmoUI.Units.longDuration
        snapMode: ListView.SnapToItem
        model: ModulesProxyModel{}
        delegate: settingsModuleDelegate
    }
}
