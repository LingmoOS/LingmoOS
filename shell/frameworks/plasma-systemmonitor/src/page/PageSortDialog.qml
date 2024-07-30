/*
 * SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>
 * SPDX-FileCopyrightText: 2023 Nate Graham <nate@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigami.delegates as KD

import org.kde.ksysguard.page as Page

Kirigami.Dialog {
    id: dialog

    property alias model : sortModel.sourceModel
    property string startPage
    property string newStartPage
    property bool startPageWasChanged: false

    preferredWidth: parent ? parent.width * 0.5 : -1
    preferredHeight: parent ? parent.height * 0.5 : -1

    focus: true

    // We already have a cancel button in the footer
    showCloseButton: false

    standardButtons: Dialog.Ok | Dialog.Cancel

    onAboutToShow: {
        sortModel.sourceModel = model
    }

    onAccepted: {
        if (dialog.startPageWasChanged) {
            startPage = newStartPage;
        }
        sortModel.applyChangesToSourceModel()
    }

    ListView {
        id: pageList

        reuseItems: true
        clip: true

        model: Page.PageSortModel {
            id: sortModel
        }
        delegate: Item {
            id: listItemContainer

            required property string title
            required property bool hidden
            required property var filesWriteable
            required property bool shouldRemoveFiles
            required property var icon
            required property int index

            width: pageList.width - pageList.leftMargin - pageList.rightMargin
            height: listItem.implicitHeight

            ItemDelegate {
                id: listItem

                width: listItemContainer.width

                activeFocusOnTab: false

                // We don't want visual interactivity for the background
                highlighted: false
                hoverEnabled: false
                down: false

                contentItem: RowLayout {
                    spacing: Kirigami.Units.smallSpacing

                    Kirigami.ListItemDragHandle {
                        id: handle
                        Layout.fillHeight: true
                        listItem: listItem
                        listView: pageList
                        onMoveRequested: (oldIndex, newIndex) => {
                            sortModel.move(oldIndex, newIndex)
                        }
                    }

                    CheckBox {
                        checked: !listItemContainer.hidden
                        onToggled: pageList.model.setData(pageList.model.index(listItemContainer.index, 0), !listItemContainer.hidden, Page.PagesModel.HiddenRole)
                        ToolTip.text: listItemContainer.hidden ? i18nc("@info:tooltip", "Show Page")
                                                               : i18nc("@info:tooltip", "Hide Page")
                        ToolTip.delay: Kirigami.Units.toolTipDelay
                        ToolTip.visible: hovered
                    }

                    KD.IconTitleSubtitle {
                        id: iconTitleSubtitle

                        Layout.fillWidth: true
                        Layout.fillHeight: true

                        icon.name: listItemContainer.icon
                        icon.width: Kirigami.Units.iconSizes.smallMedium
                        icon.height: Kirigami.Units.iconSizes.smallMedium

                        title: listItemContainer.title
                        reserveSpaceForSubtitle: true

                        opacity: listItemContainer.hidden ? 0.3 : 1
                    }

                    Button {
                        id: removeButton
                        onClicked: pageList.model.setData(pageList.model.index(listItemContainer.index, 0), !listItemContainer.shouldRemoveFiles, Page.PageSortModel.ShouldRemoveFilesRole)
                        ToolTip.delay: Kirigami.Units.toolTipDelay
                        ToolTip.visible: hovered
                    }

                    states: [
                        State {
                            name: "noChanges"
                            extend: "localChanges"
                            when: listItemContainer.filesWriteable == Page.PagesModel.NotWriteable
                            PropertyChanges {
                                target: removeButton;
                                enabled: false;
                            }
                        },
                        State {
                            name: "removeLocalChanges"
                            when:  listItemContainer.filesWriteable == Page.PagesModel.LocalChanges && listItemContainer.shouldRemoveFiles
                            PropertyChanges {
                                target: removeButton
                                icon.name: "edit-redo"
                                ToolTip.text: i18nc("@info:tooltip", "Do not reset the page")
                            }
                            PropertyChanges {
                                target: iconTitleSubtitle
                                subtitle: i18nc("@item:intable", "The page will be reset to its default state")
                            }
                        },
                        State {
                            name: "localChanges"
                            when: listItemContainer.filesWriteable == Page.PagesModel.LocalChanges
                            PropertyChanges {
                                target: removeButton
                                icon.name: "edit-reset"
                                ToolTip.text: i18nc("@info:tooltip", "Reset the page to its default state")
                            }
                        },
                        State {
                            name: "remove"
                            when: listItemContainer.filesWriteable == Page.PagesModel.AllWriteable && listItemContainer.shouldRemoveFiles
                            PropertyChanges {
                                target: removeButton
                                icon.name: "edit-undo"
                                ToolTip.text: i18nc("@info:tooltip", "Do not remove this page")
                            }
                            PropertyChanges {
                                target: iconTitleSubtitle
                                subtitle: i18nc("@item:intable", "The page will be removed")
                            }
                        },
                        State {
                            name: "removeable"
                            when: listItemContainer.filesWriteable == Page.PagesModel.AllWriteable
                            PropertyChanges {
                                target: removeButton
                                icon.name: "edit-delete"
                                ToolTip.text: i18nc("@info:tooltip", "Remove this page")
                            }
                        }
                    ]
                }

                background: Rectangle {
                    color: listItemContainer.filesWriteable == Page.PagesModel.AllWriteable && listItemContainer.shouldRemoveFiles ? Kirigami.Theme.negativeBackgroundColor : Kirigami.Theme.backgroundColor
                }
            }
        }
    }

    footerLeadingComponent: RowLayout {
        spacing: Kirigami.Units.smallSpacing

        Label {
            leftPadding: Kirigami.Units.largeSpacing
            text: i18nc("@label:listbox", "Start with:")
        }
        ComboBox {
            id: startPageBox
            textRole: "title"
            valueRole: "fileName"
            model: createModel()

            function createModel() {
                var result = [{fileName: "", title: i18n("Last Visited Page"), icon: "clock"}];
                for (var i = 0; i < sortModel.rowCount(); ++i) {
                    const index = sortModel.index(i, 0)
                    if (sortModel.data(index, Page.PagesModel.HiddenRole)) {
                        continue
                    }
                    if (sortModel.data(index, Page.PageSortModel.ShouldRemoveFilesRole)
                        && sortModel.data(index, Page.PagesModel.FilesWriteableRole) == Page.PagesModel.AllWriteable) {
                        continue
                    }
                    result.push({fileName: sortModel.data(index, Page.PagesModel.FileNameRole),
                                title: sortModel.data(index, Page.PagesModel.TitleRole),
                                icon: sortModel.data(index, Page.PagesModel.IconRole)})
                }
                return result
            }

            Connections {
                target: sortModel
                function onDataChanged() { refreshModel() }
                function onRowsMoved() { refreshModel() }
                function refreshModel() {
                    const value = startPageBox.currentValue
                    startPageBox.model = startPageBox.createModel()
                    startPageBox.currentIndex = startPageBox.indexOfValue(value)
                }
            }

            delegate: ItemDelegate {
                width: parent.width
                highlighted: startPageBox.highlightedIndex == index
                contentItem: RowLayout {
                    Kirigami.Icon {
                        color: highlighted ? Kirigami.Theme.highlightedTextColor : Kirigami.Theme.textColor
                        source: modelData.icon
                        Layout.preferredHeight: Kirigami.Units.iconSizes.small
                        Layout.preferredWidth: Kirigami.Units.iconSizes.small
                    }
                    Label {
                        text: modelData.title
                        color: highlighted ? Kirigami.Theme.highlightedTextColor : Kirigami.Theme.textColor
                        Layout.fillWidth: true
                    }
                }
            }

            onActivated: {
                dialog.startPageWasChanged = true;
                dialog.newStartPage = currentValue;
            }

            Component.onCompleted: {
                currentIndex = indexOfValue(startPage);
            }
        }
    }
}
