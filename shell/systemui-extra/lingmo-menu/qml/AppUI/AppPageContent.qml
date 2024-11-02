/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.5
import AppControls2 1.0 as AppControls2
import org.lingmo.menu.core 1.0

Item {
    property bool isAppListShow: appList.visible

    function resetFocus() {
        if (appList.visible) {
            appList.resetListFocus();
        } else {
            selectionPage.focus = true;
            labelPage.focus = true;
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 5

        AppListHeader {
            id: appListHeader
            Layout.fillWidth: true
            Layout.preferredHeight: 48
            header: AppPageBackend.appModel.header
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            AppList {
                id: appList
                anchors.fill: parent
                visible: !selectionPage.visible
                model: AppPageBackend.appModel

                view.onContentYChanged: {
                    if (view.contentY <= 0) {
                        appListHeader.title = "";
                    } else {
                        appListHeader.title = view.currentSection;
                    }
                }

                function positionLabel(label) {
                    let index = model.findLabelIndex(label);
                    if (index >= 0) {
                        view.positionViewAtIndex(index, ListView.Beginning)
                    }
                }

                onLabelItemClicked: {
                    labelPage.labelBottle = AppPageBackend.appModel.labelBottle;
                    labelPage.currentIndex = 0;
                    selectionPage.state = "viewShowed";
                }
            }

            MouseArea {
                id: selectionPage
                anchors.fill: parent
                visible: false

                onClicked: state = "viewHid";

                state: "viewHid"
                states: [
                    State {
                        name: "viewShowed"
                        changes: [
                            PropertyChanges {target: selectionPage; scale: 1; opacity: 1; focus: true }
                        ]
                    },
                    State {
                        name: "viewHid"
                        changes: [
                            PropertyChanges {target: selectionPage; scale: 1.5; opacity: 0; focus: false }
                        ]
                    }
                ]

                transitions: [
                    Transition {
                        from: "*"; to: "viewShowed"
                        SequentialAnimation {
                            ScriptAction { script: selectionPage.visible = true; }
                            NumberAnimation { properties: "scale,opacity"; easing.type: Easing.InOutCubic; duration: 300}
                        }
                    },
                    Transition {
                        from: "*"; to: "viewHid"
                        SequentialAnimation {
                            NumberAnimation { properties: "scale,opacity"; easing.type: Easing.InOutCubic; duration: 300}
                            ScriptAction {
                                script: {
                                    selectionPage.visible = false;
                                    labelPage.labelBottle = null;
                                    labelPage.focus = false;
                                    appList.focus = false;
                                }
                            }
                        }
                    }
                ]

                AppLabelPage {
                    id: labelPage
                    anchors.centerIn: parent
                    interactive: height > parent.height

                    onLabelSelected: (label) => {
                        appList.positionLabel(label);
                        selectionPage.state = "viewHid";
                    }

                    onModelChanged: {
                        currentIndex = -1;
                    }
                }

                function hidePage() {
                    state = "viewHid";
                    visible = false;
                }

                Component.onCompleted: {
                    mainWindow.visibleChanged.connect(hidePage)
                }

                Component.onDestruction: {
                    mainWindow.visibleChanged.disconnect(hidePage)
                }
            }
        }
    }
}
