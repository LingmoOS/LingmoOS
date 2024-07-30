/*
    SPDX-FileCopyrightText: 2013 Heena Mahour <heena393@gmail.com>
    SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
import QtQuick 2.0
import org.kde.lingmo.core as LingmoCore
import org.kde.lingmo.components 2.0 as LingmoComponents
import org.kde.lingmo.extras 2.0 as LingmoExtras
import org.kde.activities 0.1 as Activities

Item {
    id: main

    width: 320
    height: 320

    Row {
        id: buttons

        height: 32

        anchors {
            left:   parent.left
            right:  parent.right
            top:    parent.top
        }

        LingmoComponents.Button {
            id: buttonAdd
            text: "Add"

            onClicked: {
                modelMain.linkResourceToActivity("/tmp", function () {});
            }

            width: 64
        }

        LingmoComponents.Button {
            id: buttonRemove
            text: "Del"

            onClicked: {
                modelMain.unlinkResourceFromActivity("/tmp", function () {});
            }

            width: 64
        }

        LingmoComponents.Button {
            id: buttonSortAZ
            text: "A-Z"

            onClicked: {

                var items = [];

                for (var i = 0; i < modelMain.count(); i++) {
                    items.push([
                        modelMain.displayAt(i),
                        modelMain.resourceAt(i)
                    ]);
                }

                items = items.sort(function(left, right) {
                    return (left[0] < right[0]) ? -1 :
                           (left[0] > right[0]) ?  1 :
                                                   0
                });

                items = items.map(function(item) { return item[1]; });

                modelMain.setOrder(items);
            }

            width: 64
        }

        LingmoComponents.Button {
            id: buttonSortZA
            text: "Z-A"

            onClicked: {

                var items = [];

                for (var i = 0; i < modelMain.count(); i++) {
                    items.push([
                        modelMain.displayAt(i),
                        modelMain.resourceAt(i)
                    ]);
                }

                items = items.sort(function(left, right) {
                    return (left[0] < right[0]) ?  1 :
                           (left[0] > right[0]) ? -1 :
                                                   0
                });

                items = items.map(function(item) { return item[1]; });

                modelMain.setOrder(items);
            }

            width: 64
        }
    }

    ListView {
        id: list

        anchors {
            left: parent.left
            right: parent.right
            bottom: parent.bottom

            top: buttons.bottom

        }

        property int minimumWidth: 320
        property int minimumHeight: 320 // theme.mSize(theme.defaultFont).height * 14
        property int implicitWidth: minimumWidth * 1.5
        property int implicitHeight: minimumHeight * 1.5

        model: modelMain

        Activities.ResourceModel {
            id: modelMain
            shownAgents: "org.kde.lingmo.kickoff"
            shownActivities: ":global,:current"
        }

        add: Transition {
            NumberAnimation { properties: "x"; from: 300; duration: 1000 }
        }

        addDisplaced: Transition {
           NumberAnimation { properties: "x,y"; duration: 1000 }
        }

        remove: Transition {
            NumberAnimation { properties: "x"; to: 300; duration: 1000 }
        }

        removeDisplaced: Transition {
            NumberAnimation { properties: "x,y"; duration: 1000 }
        }

        ListModel {
            id: modelDummy

            ListElement {
                name: "Bill Smith"
                number: "555 3264"
            }
            ListElement {
                name: "John Brown"
                number: "555 8426"
            }
            ListElement {
                name: "Sam Wise"
                number: "555 0473"
            }
        }

        delegate: Column {
            height: 48
            Text {
                text: display
                height: 16
                font.bold: true
                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        if (display != "tmp") {
                            modelMain.linkResourceToActivity("/tmp", function () {});
                        } else {
                            modelMain.unlinkResourceFromActivity("/tmp", function () {});
                        }
                    }
                }

            }
            Text {
                text: "   icon: " + decoration
                height: 16
            }
            Text {
                text: "   application: " + agent
                height: 16
            }

        }
    }
}
