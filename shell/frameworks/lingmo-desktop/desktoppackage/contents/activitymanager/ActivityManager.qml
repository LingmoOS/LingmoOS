/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2014 Ivan Cukic <ivan.cukic@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.0

import org.kde.lingmo.components 3.0 as LingmoComponents
import org.kde.lingmoui 2.20 as LingmoUI
import org.kde.config as KConfig  // KAuthorized
import org.kde.kcmutils  // KCMLauncher


FocusScope {
    id: root
    signal closed()

    function parentClosed() {
        activityBrowser.parentClosed();
    }

    //this is used to perfectly align the filter field and delegates
    property int cellWidth: LingmoUI.Units.iconSizes.sizeForLabels * 30
    property int spacing: 2 * LingmoUI.Units.smallSpacing

    property bool showSwitcherOnly: false

    width: LingmoUI.Units.gridUnit * 16

    Item {
        id: activityBrowser

        property int spacing: 2 * LingmoUI.Units.smallSpacing

        signal closeRequested()

        Keys.onPressed: {
            if (event.key === Qt.Key_Escape) {
                if (heading.searchString.length > 0) {
                    heading.searchString = "";
                } else {
                    activityBrowser.closeRequested();
                }

            } else if (event.key === Qt.Key_Up) {
                activityList.selectPrevious();

            } else if (event.key === Qt.Key_Down) {
                activityList.selectNext();

            } else if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter) {
                activityList.openSelected();

            } else if (event.key === Qt.Key_Tab) {
                // console.log("TAB KEY");

            } else  {
                // console.log("OTHER KEY");
                // event.accepted = false;
                // heading.forceActiveFocus();
            }
        }

        // Rectangle {
        //     anchors.fill : parent
        //     opacity      : .4
        //     color        : "white"
        // }

        Heading {
            id: heading

            anchors {
                top: parent.top
                left: parent.left
                right: parent.right

                leftMargin: LingmoUI.Units.smallSpacing
                rightMargin: LingmoUI.Units.smallSpacing
            }

            visible: !root.showSwitcherOnly

            onCloseRequested: activityBrowser.closeRequested()
        }

        LingmoComponents.ScrollView {
            anchors {
                top:    heading.visible ? heading.bottom : parent.top
                bottom: bottomPanel.visible ? bottomPanel.top : parent.bottom
                left:   parent.left
                right:  parent.right
                topMargin: activityBrowser.spacing
            }

            ActivityList {
                id: activityList
                showSwitcherOnly: root.showSwitcherOnly
                filterString: heading.searchString.toLowerCase()
                itemsWidth: root.width - LingmoUI.Units.smallSpacing
            }
        }

        Item {
            id: bottomPanel

            height: newActivityButton.height + LingmoUI.Units.gridUnit

            visible: !root.showSwitcherOnly

            anchors {
                bottom: parent.bottom
                left: parent.left
                right: parent.right
            }

            LingmoComponents.ToolButton {
                id: newActivityButton

                text: i18nd("lingmo_shell_org.kde.lingmo.desktop", "Create activity…")
                icon.name: "list-add"

                width: parent.width

                onClicked: KCMLauncher.openSystemSettings("kcm_activities", "newActivity")

                visible: KConfig.KAuthorized.authorize("lingmo-desktop/add_activities")
                opacity: newActivityDialog.status == Loader.Ready ?
                              1 - newActivityDialog.item.opacity : 1
            }

            Loader {
                id: newActivityDialog

                z: 100

                anchors.bottom: newActivityButton.bottom
                anchors.left:   newActivityButton.left
                anchors.right:  newActivityButton.right

            }
        }

        onCloseRequested: root.closed()

        anchors.fill: parent
    }

}

