/*
 *  SPDX-FileCopyrightText: 2020 Łukasz Korbel <corebell.it@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick
import QtQuick.Controls 2.5 as QQC2
import QtQuick.Layouts 1.0
import org.kde.kirigami 2.20 as Kirigami
import org.kde.plasma.plasmoid 2.0
import org.kde.kcmutils as KCM

KCM.ScrollViewKCM {
    id: timesPage
    property var cfg_predefinedTimers: []
    readonly property int maxListSize: 15

    signal configurationChanged()

    Component.onCompleted: {
        for (var i of plasmoid.configuration.predefinedTimers) {
            timeListModel.append({"time": i})
        }
    }

    ListModel {
        id: timeListModel
        function addTimer(value) {
            timeListModel.append({"time": value})
            cfg_predefinedTimers.splice(count, 0, value)
            timesPage.configurationChanged()
        }
        function removeTimer(index) {
            remove(index)
            cfg_predefinedTimers.splice(index, 1)
            timesPage.configurationChanged()
        }
        function moveTimer(oldIndex, newIndex) {
            move(oldIndex, newIndex, 1)
            cfg_predefinedTimers[oldIndex] = get(oldIndex).time
            cfg_predefinedTimers[newIndex] = get(newIndex).time
            timesPage.configurationChanged()
        }
        function setTimer(index, newValue) {
            setProperty(index, "time", newValue)
            cfg_predefinedTimers[index] = newValue
            timesPage.configurationChanged()
        }
    }

    view: ListView {
        id: timeListView
        anchors.margins: 4
        model: timeListModel
        spacing: parent.spacing
        clip: true
        reuseItems: true
        delegate: timeEditDelegate
        add: Transition {
            NumberAnimation { properties: "y"; duration: Kirigami.Units.longDuration }
        }
        displaced: Transition {
            NumberAnimation { properties: "y"; duration: Kirigami.Units.longDuration }
        }

        Kirigami.PlaceholderMessage {
            visible: timeListView.count === 0
            anchors.centerIn: parent
            width: parent.width - (Kirigami.Units.gridUnit * 4)
            text: i18n("If you add predefined timers here, they will appear in plasmoid context menu.");
        }
    }

    footer: RowLayout {
        QQC2.Button {
            icon.name: "list-add"
            text: i18n("Add")
            enabled: timeListModel.count < maxListSize
            onClicked: {
                timeListModel.addTimer("0")
                timeListView.positionViewAtEnd()
            }
        }
    }

    Component {
        id: timeEditDelegate
        Kirigami.SwipeListItem {
            id: timeEditItem
            width: timeListView.width
            contentItem: Row {
                spacing: Kirigami.Units.gridUnit
                Kirigami.ListItemDragHandle {
                    anchors.verticalCenter: parent.verticalCenter
                    enabled: !editor.editable
                    listItem: timeEditItem
                    listView: timeListView
                    onMoveRequested: (oldIndex, newIndex) => {
                        timeListModel.moveTimer(oldIndex, newIndex)
                    }
                }
                TimerEdit {
                    id: editor
                    alertMode: editable
                    value: time
                    property int oldValue: 0
                    onDigitModified: valueDelta => {
                        set(value + valueDelta)
                    }
                    function set(newValue) {
                        timeListModel.setTimer(index, (newValue).toString())
                    }
                    Component.onCompleted: editable = (value === 0)
                }
                QQC2.Label {
                    id: hintText
                    visible: editor.editable
                    text: i18n("Scroll over digits to change time")
                    textFormat: Text.PlainText
                    anchors.verticalCenter: parent.verticalCenter
                }
            }
            actions: [
                Kirigami.Action {
                    text: i18n("Apply")
                    icon.name: "dialog-ok-apply"
                    visible: editor.editable
                    onTriggered: {
                        editor.editable = false
                    }
                },
                Kirigami.Action {
                    text: i18n("Cancel")
                    icon.name: "dialog-cancel"
                    visible: editor.editable
                    onTriggered: {
                        editor.editable = false
                        editor.set(editor.oldValue)
                    }
                },
                Kirigami.Action {
                    text: i18n("Edit")
                    icon.name: "edit-entry"
                    visible: editor.editable === false
                    onTriggered: {
                        editor.oldValue = editor.value
                        editor.editable = true
                    }
                },
                Kirigami.Action {
                    text: i18n("Delete")
                    icon.name: "entry-delete"
                    visible: editor.editable === false
                    onTriggered: {
                        timeListModel.removeTimer(index)
                    }
                }
            ]
        }
    }
}

