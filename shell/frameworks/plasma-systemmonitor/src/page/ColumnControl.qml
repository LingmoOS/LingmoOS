/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

import org.kde.ksysguard.page

Container {
    id: control

    property PageDataObject columnData

    function replaceSensors(replacement) {
        for (let i = 0; i < repeater.count; ++i) {
            repeater.itemAt(i).replaceSensors(replacement)
        }
    }

    Kirigami.AbstractCard {
        parent: control.background
        anchors.fill: parent
        visible: control.columnData.showBackground
        showClickFeedback: true
        onClicked: control.select(control)
    }

    contentItem: Row {
        id: row

        anchors.fill: parent
        anchors.topMargin: control.topPadding
        anchors.bottomMargin: control.bottomPadding
        anchors.leftMargin: control.leftPadding
        anchors.rightMargin: control.rightPadding

        spacing: Kirigami.Units.largeSpacing

        move: Transition {
            NumberAnimation { properties: "x,y"; duration: Kirigami.Units.shortDuration }
        }

        function relayout() {
            let itemCount = repeater.count;
            let separatorsWidth = 0;
            let separatorsCount = 0;
            let minimumContentHeight = 0;
            let minimumHeight = 0;

            for (let i = 0; i < itemCount; ++i) {
                let item = repeater.itemAt(i)
                if (item) {
                    if (item.sectionData.isSeparator) {
                        separatorsWidth += item.width
                        separatorsCount += 1
                    } else {
                        minimumContentHeight = Math.max(minimumContentHeight, item.minimumContentHeight)
                        minimumHeight = Math.max(minimumHeight, item.minimumHeight)
                    }
                }
            }

            let sectionWidth = (row.width - separatorsWidth - (itemCount - 1) * spacing) / (itemCount - separatorsCount)

            for (let i = 0; i < itemCount; ++i) {
                let item = repeater.itemAt(i)
                if (item && !item.sectionData.isSeparator) {
                    item.width = sectionWidth
                }
            }

            control.minimumContentHeight = minimumContentHeight
            control.minimumHeight = minimumHeight + control.topPadding + control.bottomPadding
        }

        onWidthChanged: Qt.callLater(relayout)
        onHeightChanged: Qt.callLater(relayout)

        Repeater {
            id: repeater
            model: PageDataModel { data: control.columnData }

            onItemAdded: Qt.callLater(row.relayout)
            onItemRemoved: Qt.callLater(row.relayout)

            SectionControl {
                height: parent.height
                onWidthChanged: Qt.callLater(row.relayout)

                onMinimumContentHeightChanged: Qt.callLater(row.relayout)
                onMinimumHeightChanged: Qt.callLater(row.relayout)

                activeItem: control.activeItem
                single: control.columnData.children.length == 1
                sectionData: model.data
                index: model.index

                onSelect: item => control.select(item)
                onAddSection: control.addSection(index + 1)
                onAddSeparator: control.addSeparator(index + 1)
                onRemove: control.columnData.removeChild(index)
                onMove: (from, to) => control.columnData.moveChild(from, to)

                onMissingSensorsChanged: (id, title, sensors) => control.missingSensorsChanged(id, title, sensors)
            }
        }
    }

    toolbar.addActions: [
        Action {
            text: i18nc("@action", "Add Section")
            onTriggered: addSection(control.columnData.children.length)
        },
        Action {
            text: i18nc("@action", "Add Separator")
            onTriggered: addSeparator(control.columnData.children.length)
        }
    ]
    toolbar.moveAxis: Qt.XAxis

    toolbar.extraActions: [
        Action {
            icon.name: "view-visible"
            text: i18nc("@action", "Show Background")
            checkable: true
            checked: control.columnData.showBackground
            onTriggered: control.columnData.showBackground = !control.columnData.showBackground
        },
        Action {
            icon.name: "trim-margins"
            text: i18nc("@action", "Remove Background Margins")
            enabled: control.columnData.showBackground
            checkable: true
            checked: control.columnData.noMargins
            onTriggered: control.columnData.noMargins = !control.columnData.noMargins
        }
    ]

    function addSection(index) {
        control.columnData.insertChild(index, {name: "section-" + index, isSeparator: false})
    }

    function addSeparator(index) {
        control.columnData.insertChild(index, {name: "section-" + index, isSeparator: true})
    }

    Component.onCompleted: {
        if (columnData.children.length == 0) {
            addSection(0)
        }
    }
}
