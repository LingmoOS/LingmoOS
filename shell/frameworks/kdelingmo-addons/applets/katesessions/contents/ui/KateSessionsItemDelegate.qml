/*
    SPDX-FileCopyrightText: 2014 Joseph Wenninger <jowenn@kde.org>
    SPDX-FileCopyrightText: 2022 Alexander Lohnau <alexander.lohnau@gmx.de>

    Based on the clipboard applet:
    SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
    SPDX-FileCopyrightText: 2014 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
import QtQuick 2.0
import QtQuick.Layouts 1.1
import org.kde.lingmo.components 3.0 as LingmoComponents3
import org.kde.kquickcontrolsaddons 2.0 as KQuickControlsAddons
import org.kde.lingmoui 2.20 as LingmoUI

import org.kde.lingmo.private.profiles 1.0

LingmoComponents3.ItemDelegate {
    id: menuItem

    signal itemSelected(string profileIdentifier)

    property bool showInput: false

    height: Math.max(label.height, sessionnameditlayout.implicitHeight) + LingmoUI.Units.smallSpacing

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true

        onClicked: {
            if (profileIdentifier !== "")
                menuItem.itemSelected(profileIdentifier);
            else {
                showInput=true;
                sessionname.forceActiveFocus(Qt.MouseFocusReason);
            }
        }
        onEntered: menuListView.currentIndex = index
        onExited: menuListView.currentIndex = -1

        Item {
            id: label
            height: iconItem.height
            anchors {
                left: parent.left
                leftMargin: LingmoUI.Units.smallSpacing
                right: parent.right
                verticalCenter: parent.verticalCenter
            }

            LingmoComponents3.Label {
                anchors {
                    left: parent.left
                    right: parent.right
                    rightMargin: LingmoUI.Units.gridUnit * 2
                    leftMargin: LingmoUI.Units.iconSizes.small + LingmoUI.Units.smallSpacing * 2
                    verticalCenter: parent.verticalCenter
                }
                maximumLineCount: 1
                text: name.trim()
                textFormat: Text.PlainText
                visible: !showInput
                elide: Text.ElideRight
                wrapMode: Text.Wrap
            }

            LingmoUI.Icon {
                id: iconItem
                width: LingmoUI.Units.iconSizes.small
                height: width
                anchors.verticalCenter: parent.verticalCenter
                source: iconName
            }
        }

        RowLayout {
                id:sessionnameditlayout
                visible:showInput
                height: implicitHeight
                anchors {
                    left: parent.left
                    right: parent.right
                    rightMargin: 0
                    leftMargin: LingmoUI.Units.iconSizes.small + LingmoUI.Units.smallSpacing * 2
                    verticalCenter: parent.verticalCenter
                }

                LingmoComponents3.TextField {
                    id: sessionname
                    placeholderText: i18n("Session name")
                    clearButtonShown: true
                    Layout.fillWidth: true
                    Keys.onReturnPressed: {menuItem.itemSelected(sessionname.text.replace(/^\s+|\s+$/g, '')); showInput=false;}
                }

                LingmoComponents3.ToolButton {
                    icon.name: "dialog-ok"
                    enabled: sessionname.text.replace(/^\s+|\s+$/g, '').length>0
                    onClicked: {menuItem.itemSelected(sessionname.text.replace(/^\s+|\s+$/g, '')); showInput=false;}

                    LingmoComponents3.ToolTip {
                        text: i18n("Create new session and start Kate")
                    }
                }

                LingmoComponents3.ToolButton {
                    icon.name: "dialog-cancel"
                    onClicked: {
                        showInput=false;
                        sessionname.text='';
                    }

                    LingmoComponents3.ToolTip {
                        text: i18n("Cancel session creation")
                    }
                }
        }
    }
}
