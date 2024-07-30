/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 * SPDX-FileCopyrightText: 2023 Nate Graham <nate@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

import org.kde.ksysguard.formatter as Formatter
import org.kde.ksysguard.table as Table

Kirigami.Dialog {
    id: columnDialog

    property alias model: columnView.model
    property alias sourceModel: sortModel.sourceModel
    property var visibleColumns: ["name"]
    property var sortedColumns: []
    property var fixedColumns: []
    property var columnDisplay: {"name": "text"}

    title: i18ndc("plasma-systemmonitor", "@window:title", "Configure Columns")

    preferredWidth: parent ? parent.width * 0.75 : 0
    preferredHeight: parent ? parent.height * 0.75 : 0

    focus: true

    // We already have a cancel button in the footer
    showCloseButton: false

    standardButtons: Dialog.Ok | Dialog.Cancel

    function setColumnDisplay(display) {
        columnDisplay = display
        prepare()
        apply()
    }

    onAccepted: {
        apply()
    }

    onAboutToShow: {
        prepare()
    }

    function prepare() {
        sortModel.sortedColumns = sortedColumns.filter(id => fixedColumns.indexOf(id) == -1)
        let tempDisplay = columnDisplay
        fixedColumns.forEach(column => delete tempDisplay[column])
        displayModel.columnDisplay = tempDisplay
    }

    function apply() {
        sortedColumns = fixedColumns.concat(sortModel.sortedColumns)
        visibleColumns = fixedColumns.concat(displayModel.visibleColumnIds)
        columnDisplay = displayModel.columnDisplay
    }

    function hideColumn(columnId) {
        displayModel.setDisplayById(columnId, "hidden");
        accept()
    }

    ListView {
        id: columnView

        Kirigami.Theme.colorSet: Kirigami.Theme.View
        Kirigami.Theme.inherit: false

        clip: true

        model: Table.ColumnDisplayModel {
            id: displayModel

            sourceModel: Table.ColumnSortModel {
                id: sortModel
            }
        }

        delegate: Loader {
            width: columnView.width
            height: Kirigami.Units.gridUnit * 3
            property var modelData: model
            sourceComponent: delegateComponent
        }

        Component {
            id: delegateComponent
            Kirigami.SubtitleDelegate {
                id: delegate
                rightPadding: Kirigami.Units.smallSpacing
                property int index: modelData ? modelData.row : -1
                activeFocusOnTab: false

                highlighted: parent === columnView

                // We don't want visual interactivity for the background except on drag.
                hoverEnabled: false
                down: false

                text: modelData?.name ?? ""
                subtitle: modelData?.description ?? ""

                Kirigami.Theme.useAlternateBackgroundColor: true

                ToolTip.visible: hoverHandler.hovered && contentItem.truncated

                contentItem: RowLayout {
                    spacing: Kirigami.Units.smallSpacing

                    property bool truncated: titleSubtitle.truncated

                    Kirigami.ListItemDragHandle {
                        id: handle
                        listItem: delegate
                        listView: columnView
                        onMoveRequested: (oldIndex, newIndex) => {
                            sortModel.move(oldIndex, newIndex);
                        }
                    }

                    Kirigami.TitleSubtitle {
                        id: titleSubtitle
                        Layout.fillWidth: true
                        title: delegate.text
                        subtitle: delegate.subtitle
                    }

                    ComboBox {
                        id: showCombo
                        textRole: "text"
                        Layout.rowSpan: 2
                        Layout.rightMargin: Kirigami.Units.smallSpacing
                        model: {
                            var result = [
                                {text: i18ndc("plasma-systemmonitor", "@item:inlistbox", "Hidden"), value: "hidden"},
                                {text: i18ndc("plasma-systemmonitor", "@item:inlistbox", "Text Only"), value: "text"},
                            ]

                            if (modelData && modelData.unit) {
                                if (modelData.unit != Formatter.Units.UnitInvalid
                                    && modelData.unit != Formatter.Units.UnitNone) {
                                    result.push({text: i18ndc("plasma-systemmonitor", "@item:inlistbox", "Line Chart"), value: "line"})
                                }
                                if (modelData.unit == Formatter.Units.UnitPercent
                                    && modelData.maximum != 100) {
                                    result.push({text: i18ndc("plasma-systemmonitor", "@item:inlistbox", "Text Only (Scaled to 100%)"), value: "textScaled"})
                                    result.push({text: i18ndc("plasma-systemmonitor", "@item:inlistbox", "Line Chart (Scaled to 100%)"), value: "lineScaled"})
                                }
                            }
                            return result
                        }

                        currentIndex: {
                            if (!modelData) {
                                return -1;
                            }

                            for (var i = 0; i < model.length; ++i) {
                                if (model[i].value == modelData.displayStyle) {
                                    return i;
                                }
                            }
                            return -1;
                        }

                        onActivated: {
                            displayModel.setDisplay(delegate.index, model[index].value);
                        }
                    }
                }

                HoverHandler {
                    id: hoverHandler
                }
            }
        }
    }
}
