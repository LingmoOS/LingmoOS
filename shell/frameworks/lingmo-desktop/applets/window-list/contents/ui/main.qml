/*
    SPDX-FileCopyrightText: 2016 Eike Hein <hein@kde.org>
    SPDX-FileCopyrightText: 2022 Carson Black <uhhadd@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.10

import org.kde.lingmo.plasmoid 2.0
import org.kde.lingmo.core as LingmoCore
import org.kde.lingmo.components as LingmoComponents
import org.kde.lingmo.extras 2.0 as LingmoExtras
import org.kde.lingmoui 2.20 as LingmoUI

import org.kde.taskmanager 0.1 as TaskManager

PlasmoidItem {
    Plasmoid.constraintHints: Plasmoid.CanFillArea
    compactRepresentation: windowListButton
    fullRepresentation: windowListView
    switchWidth: LingmoUI.Units.gridUnit * 8
    switchHeight: LingmoUI.Units.gridUnit * 6

    Component {
        id: windowListView

        ListView {
            clip: true
            Layout.preferredWidth: LingmoUI.Units.gridUnit * 10
            Layout.preferredHeight: LingmoUI.Units.gridUnit * 12
            model: TaskManager.TasksModel {
                id: tasksModel

                sortMode: TaskManager.TasksModel.SortVirtualDesktop
                groupMode: TaskManager.TasksModel.GroupDisabled
            }
            delegate: LingmoComponents.ItemDelegate {

                width: ListView.view.width

                contentItem: RowLayout {
                    spacing: LingmoUI.Units.smallSpacing

                    LingmoUI.Icon {
                        id: iconItem

                        source: model.decoration
                        visible: source !== "" && iconItem.valid

                        implicitWidth: LingmoUI.Units.iconSizes.sizeForLabels
                        implicitHeight: LingmoUI.Units.iconSizes.sizeForLabels
                    }
                    // Fall back to a generic icon if the application doesn't provide a valid one
                    LingmoUI.Icon {
                        source: "preferences-system-windows"
                        visible: !iconItem.valid

                        implicitWidth: LingmoUI.Units.iconSizes.sizeForLabels
                        implicitHeight: LingmoUI.Units.iconSizes.sizeForLabels
                    }
                    LingmoComponents.Label {
                        Layout.fillWidth: true
                        text: model.display
                        textFormat: Text.PlainText
                        elide: Text.ElideRight
                    }
                }

                onClicked: tasksModel.requestActivate(tasksModel.makeModelIndex(model.index))
            }
        }
    }

    // Only exists because the default CompactRepresentation doesn't expose the
    // ability to show text below or beside the icon.
    // TODO remove once it gains that feature.
    Component {
        id: windowListButton

        MenuButton {
            Layout.minimumWidth: implicitWidth
            Layout.maximumWidth: implicitWidth
            Layout.fillHeight: Plasmoid.formFactor === LingmoCore.Types.Horizontal
            Layout.fillWidth: Plasmoid.formFactor === LingmoCore.Types.Vertical

            onClicked: tasksMenu.open()
            down: pressed || tasksMenu.status === LingmoExtras.Menu.Open

            Accessible.name: Plasmoid.title
            Accessible.description: toolTipSubText

            text: if (tasksModel.activeTask.valid) {
                return tasksModel.data(tasksModel.activeTask, TaskManager.AbstractTasksModel.AppName) ||
                       tasksModel.data(tasksModel.activeTask, 0 /* display name, window title if app name not present */)
            } else {
                return i18n("Lingmo Desktop")
            }

            iconSource: if (tasksModel.activeTask.valid) {
                return tasksModel.data(tasksModel.activeTask, 1 /* decorationrole */)
            } else {
                return "start-here-kde-symbolic"
            }

            TaskManager.TasksModel {
                id: tasksModel

                sortMode: TaskManager.TasksModel.SortVirtualDesktop
                groupMode: TaskManager.TasksModel.GroupDisabled
            }

            LingmoExtras.ModelContextMenu {
                id: tasksMenu

                placement: {
                   if (Plasmoid.location === LingmoCore.Types.LeftEdge) {
                       return LingmoCore.Types.RightPosedTopAlignedPopup
                   } else if (Plasmoid.location === LingmoCore.Types.TopEdge) {
                       return LingmoCore.Types.BottomPosedLeftAlignedPopup
                   } else if (Plasmoid.location === LingmoCore.Types.RightEdge) {
                       return LingmoCore.Types.LeftPosedTopAlignedPopup
                   } else {
                       return LingmoCore.Types.TopPosedLeftAlignedPopup
                   }
                }

                model: tasksModel
                onClicked: (model) =>
                    tasksModel.requestActivate(tasksModel.makeModelIndex(model.index))
            }
        }
    }
}
