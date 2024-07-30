/**
 * SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import QtQuick.Dialogs as QtDialogs

import org.kde.lingmoui as LingmoUI
import org.kde.lingmo.kcm.flatpakpermissions

LingmoUI.PromptDialog {
    id: root

    required property FlatpakPermissionModel model
    required property /*FlatpakPermissionsSectionType::Type*/ int sectionType

    title: switch (sectionType) {
        case FlatpakPermissionsSectionType.Filesystems: return i18n("Add Filesystem Path Permission");
        case FlatpakPermissionsSectionType.SessionBus: return i18n("Add Session Bus Permission");
        case FlatpakPermissionsSectionType.SystemBus: return i18n("Add System Bus Permission");
        default: return ""
    }

    standardButtons: QQC2.Dialog.Ok | QQC2.Dialog.Discard
    closePolicy: QQC2.Popup.CloseOnEscape

    QQC2.Overlay.modal: KcmPopupModal {}

    LingmoUI.FormLayout {
        RowLayout {
            spacing: LingmoUI.Units.smallSpacing
            Layout.fillWidth: true

            LingmoUI.FormData.label: switch (root.sectionType) {
                case FlatpakPermissionsSectionType.Filesystems: return i18n("Enter filesystem path…");
                case FlatpakPermissionsSectionType.SessionBus: return i18n("Enter session bus name…");
                case FlatpakPermissionsSectionType.SystemBus: return i18n("Enter system bus name…");
                default: return ""
            }

            QQC2.TextField {
                id: nameField

                Layout.fillWidth: true

                Keys.onEnterPressed: valueBox.forceActiveFocus(Qt.TabFocusReason)
                Keys.onReturnPressed: valueBox.forceActiveFocus(Qt.TabFocusReason)
                KeyNavigation.down: valueBox
                KeyNavigation.right: selectButton
            }

            QQC2.Button {
                id: selectButton

                KeyNavigation.down: valueBox

                visible: root.sectionType === FlatpakPermissionsSectionType.Filesystems
                // TODO: add description before next string freeze
                icon.name: "document-open"

                onClicked: root.openFileDialog();
            }
        }

        QQC2.ComboBox {
            id: valueBox

            Layout.fillWidth: true

            Keys.onEnterPressed: maybeAccept()
            Keys.onReturnPressed: maybeAccept()

            function maybeAccept() {
                if (!popup.opened) {
                    root.accepted();
                }
            }

            model: root.model.valuesModelForSectionType(root.sectionType)
            textRole: "display"
            valueRole: "value"
        }
    }

    // created once, on demand
    property QtDialogs.FolderDialog __dialog

    readonly property Component __dialogComponent: Component {
        id: dialogComponent

        // TODO: make 2 buttons. One for selecting files, other one for
        // folders. It appears to be impossible to force the more
        // convenient file picker to select folders too.
        QtDialogs.FolderDialog {
            onAccepted: {
                let path = selectedFolder.toString();
                const schema = "file://";
                if (path.startsWith(schema)) {
                    path = path.slice(schema.length);
                }
                nameField.text = path;
            }
        }
    }

    function openFileDialog() {
        if (!__dialog) {
            __dialog = dialogComponent.createObject(root);
        }
        __dialog.selectedFolder = nameField.text;
        __dialog.open();
    }

    function acceptableInput() {
        const name = nameField.text;

        if (permsModel.permissionExists(sectionType, name)) {
            return false;
        }

        switch (sectionType) {
        case FlatpakPermissionsSectionType.Filesystems:
            return permsModel.isFilesystemNameValid(name);
        case FlatpakPermissionsSectionType.SessionBus:
        case FlatpakPermissionsSectionType.SystemBus:
            return permsModel.isDBusServiceNameValid(name);
        default:
            return false;
        }
    }

    onOpened: {
        const ok = standardButton(QQC2.Dialog.Ok);
        ok.enabled = Qt.binding(() => acceptableInput());
        ok.KeyNavigation.up = valueBox;

        const discard = standardButton(QQC2.Dialog.Discard);
        discard.KeyNavigation.up = valueBox;

        nameField.forceActiveFocus(Qt.PopupFocusReason);
    }

    onAccepted: {
        if (acceptableInput()) {
            const name = nameField.text;
            const value = valueBox.currentValue;
            permsModel.addUserEnteredPermission(sectionType, name, value);
            close();
        }
    }

    onDiscarded: {
        close();
    }
}
