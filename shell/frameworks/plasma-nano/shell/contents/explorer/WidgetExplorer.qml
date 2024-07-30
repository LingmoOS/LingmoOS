/*
 *   SPDX-FileCopyrightText: 2011 Marco Martin <mart@kde.org>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick 2.2
import QtQuick.Layouts 1.4

import org.kde.plasma.components 3.0 as PC3
import org.kde.kirigami 2.20 as Kirigami

import QtQuick.Window 2.1

import org.kde.plasma.private.shell 2.0

PC3.Drawer {
    id: root

    property QtObject containment
    property Item containmentInterface
    readonly property bool horizontal: containmentInterface.width <= containmentInterface.height
    readonly property int delegateSize: Kirigami.Units.gridUnit * 8

    property int topPanelHeight
    property int bottomPanelHeight
    property int leftPanelWidth
    property int rightPanelWidth

    visible: true

    width: horizontal ? containmentInterface.screenGeometry.width : implicitWidth + leftPadding
    height: horizontal ? implicitHeight + bottomPadding : containmentInterface.screenGeometry.height
    edge: horizontal ? Qt.BottomEdge : Qt.LeftEdge

    leftPadding: containmentInterface.availableScreenRect.x
    topPadding: horizontal ? Kirigami.Units.smallSpacing : containmentInterface.availableScreenRect.y
    rightPadding: horizontal ? containmentInterface.screenGeometry.width - containmentInterface.availableScreenRect.width - containmentInterface.availableScreenRect.x : 0
    bottomPadding: containmentInterface.screenGeometry.height - containmentInterface.availableScreenRect.height - containmentInterface.availableScreenRect.y

    implicitWidth: categoriesView.shouldBeVisible ? layout.implicitWidth : view.implicitWidth + Kirigami.Units.smallSpacing
    implicitHeight: categoriesView.shouldBeVisible ? layout.implicitHeight : view.implicitHeight + Kirigami.Units.smallSpacing

    Behavior on implicitWidth {
        NumberAnimation {
            duration: Kirigami.Units.longDuration
            easing.type: Easing.InOutQuad
        }
    }
    Behavior on implicitHeight {
        NumberAnimation {
            duration: Kirigami.Units.longDuration
            easing.type: Easing.InOutQuad
        }
    }
    contentItem: Item {
        implicitWidth: layout.implicitWidth
        implicitHeight: layout.implicitHeight
        clip: false

        PC3.RoundButton {
            z: 1
            anchors.bottom: parent.bottom
            x: root.horizontal ? parent.width - width : 0
            width: Kirigami.Units.iconSizes.large
            height: width

            icon.name: "view-filter"
            checked: categoriesView.shouldBeVisible
            onClicked: categoriesView.shouldBeVisible = !categoriesView.shouldBeVisible
        }

        GridLayout {
            id: layout
            anchors {
                top: parent.top
                right: parent.right

                bottom: root.horizontal ? undefined : parent.bottom
                left: root.horizontal ? parent.left : undefined
            }
            rows: root.horizontal ? 2 : 1
            columns: root.horizontal ? 1 : 2
            
            PC3.ScrollView {
                id: categoriesView
                property bool shouldBeVisible: false
                clip: false
                Layout.fillWidth: root.horizontal
                Layout.fillHeight: !root.horizontal
                Layout.column: 0
                Layout.row: root.horizontal ? 1 : 0
                implicitHeight: Kirigami.Units.gridUnit * 2
                implicitWidth: Kirigami.Units.gridUnit * 8
                opacity: categoriesView.shouldBeVisible

                Behavior on opacity {
                    NumberAnimation {
                        duration: Kirigami.Units.longDuration
                        easing.type: Easing.InOutQuad
                    }
                }

                ListView {
                    clip: false
                    model: widgetExplorer.filterModel
                    orientation: root.horizontal ? ListView.Horizontal : ListView.Vertical
                    topMargin: root.horizontal ? 0 : root.topPanelHeight
                    delegate: PC3.ItemDelegate {
                        height: model.separator ? 1 : implicitHeight
                        width: root.horizontal ? implicitWidth : parent.width
                        text: model.separator ? "" : model.display
                        checked: widgetExplorer.widgetsModel.filterType == model.filterType && widgetExplorer.widgetsModel.filterQuery == model.filterData

                        onClicked: {
                            widgetExplorer.widgetsModel.filterQuery = model.filterData
                            widgetExplorer.widgetsModel.filterType = model.filterType
                        }
                    }
                }
            }

            PC3.ScrollView {
                id: view
                clip: false
                Layout.fillWidth: root.horizontal
                Layout.fillHeight: !root.horizontal
                Layout.row: 0
                Layout.column: root.horizontal ? 0 : 1
                implicitWidth: delegateSize + Kirigami.Units.gridUnit
                implicitHeight: delegateSize + Kirigami.Units.gridUnit * 3

                ListView {
                    id: appletsList
                    clip: false
                    topMargin: root.horizontal ? 0 : root.topPanelHeight
                    header: Kirigami.Heading {
                        text: i18n("Widgets")
                        visible: !root.horizontal
                        width: visible ? implicitWidth : 0
                        height: visible ? implicitHeight : 0
                    }
                    orientation: root.horizontal ? ListView.Horizontal : ListView.Vertical
                    model: widgetExplorer.widgetsModel
                    delegate: AppletDelegate {}
                    Component.onCompleted: appletsList.contentY = -appletsList.topMargin - headerItem.height
                }
            }
        }
    }

    WidgetExplorer {
        id: widgetExplorer
        containment: root.containment
        //view: desktop
        onShouldClose: root.close();
    }
}
