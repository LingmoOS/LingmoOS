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
import org.kde.kitemmodels as KItemModels
import org.kde.iconthemes as KIconThemes

import org.kde.ksysguard.page

Kirigami.Dialog {
    id: dialog

    property string acceptText: i18nc("@action:button", "Add")
    property string acceptIcon: "list-add"

    property string name: i18nc("@info:placeholder", "New Page")
    property string iconName: "ksysguardd"
    property real margin: 2
    property string actionsFace: "dummy" //Can't leave it empty, otherwise it doesn't update when it's initially set to an empty string
    property alias pageData: facesModel.pageData

    property string loadType: "ondemand"

    // No top padding since the content item is a Kirigami.FormLayout which
    // brings its own. But it doesn't provide any other padding, so we need to
    // add some or else it touches the edges of the dialog's content area,
    // which looks bad.
    topPadding: 0
    leftPadding: Kirigami.Units.largeSpacing
    rightPadding: Kirigami.Units.largeSpacing
    bottomPadding: Kirigami.Units.largeSpacing

    focus: true

    // We already have a cancel button in the footer
    showCloseButton: false

    standardButtons: Kirigami.Dialog.Cancel

    customFooterActions: [
        Kirigami.Action {
            text: dialog.acceptText
            icon.name: dialog.acceptIcon
            enabled: nameField.acceptableInput
            onTriggered: {
                if (actionsCombobox.currentValue) {
                    dialog.actionsFace = actionsCombobox.currentValue;
                }
                dialog.accept();
            }
        }
    ]

    onOpened: {
        // Reset focus to the name field.
        // When opening the dialog multiple times, the focus object remains the
        // last focussed item, which is usually one of the buttons. Since the
        // first things we generally want to do in this dialog is edit the
        // title, we reset the focus to the name field when the dialog is
        // opened.
        nameField.forceActiveFocus()
    }

    onClosed: {
        actionsFace = "dummy" //see above
        pageData = null
    }

    Kirigami.FormLayout {
        id: form

        focus: true

        TextField {
            id: nameField

            Kirigami.FormData.label: i18nc("@label:textbox", "Name:")
            text: dialog.name
            onTextEdited: dialog.name = text

            focus: true

            validator: RegularExpressionValidator { regularExpression: /^[\pL\pN][^\pC/]*/ }
        }
        Kirigami.InlineMessage {
            Layout.fillWidth: true
            type: Kirigami.MessageType.Error
            text: i18nc("@info:status", "Name cannot be empty.")
            visible: !nameField.acceptableInput
        }
        Button {
            Kirigami.FormData.label: i18nc("@label:textbox", "Icon:")
            icon.name: dialog.iconName
            onClicked: iconDialog.open()
        }
        ComboBox {
            Kirigami.FormData.label: i18nc("@label:listbox", "Page Margins:")

            textRole: "key"

            currentIndex: {
                for (var i in model) {
                    if (model[i].value == dialog.margin) {
                        return i
                    }
                }
            }

            model: [
                { value: 0, key: i18nc("@item:inlistbox", "None") },
                { value: 1, key: i18nc("@item:inlistbox", "Small") },
                { value: 2, key: i18nc("@item:inlistbox", "Large") }
            ]

            onActivated: index => {
                dialog.margin = model[index].value;
            }
        }
        ComboBox {
            id: actionsCombobox
            Kirigami.FormData.label: i18nc("@label:listbox", "Use Actions From:")
            visible: count > 1
            textRole: "display"
            valueRole: "id"
            currentIndex:indexOfValue(dialog.actionsFace)
            model:  KItemModels.KSortFilterProxyModel {
                sourceModel: FacesModel {id: facesModel}
                filterRowCallback: function(row, parent) {
                    // No face option
                    if (row == facesModel.rowCount() - 1) {
                        return true
                    }
                    const face = facesModel.faceAtIndex(row)
                    return face.primaryActions.length != 0 && face.secondaryActions.length != 0
                }
            }
        }
        ComboBox {
            Kirigami.FormData.label: i18nc("@label:listbox", "Load this page:")

            textRole: "key"

            currentIndex: {
                for (var i in model) {
                    if (model[i].value == dialog.loadType) {
                        return i
                    }
                }
            }

            model: [
                { value: "ondemand", key: i18nc("@item:inlistbox", "When needed") },
                { value: "onstart", key: i18nc("@item:inlistbox", "During application startup") }
            ]

            onActivated: index => {
                dialog.loadType = model[index].value;
            }
        }
    }

    KIconThemes.IconDialog {
        id: iconDialog
        iconSize: Kirigami.Units.iconSizes.smallMedium
        onIconNameChanged: dialog.iconName = iconName
    }
}
