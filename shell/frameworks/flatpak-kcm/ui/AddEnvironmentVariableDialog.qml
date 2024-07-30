/**
 * SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.lingmoui as LingmoUI
import org.kde.lingmo.kcm.flatpakpermissions

LingmoUI.PromptDialog {
    id: root

    required property FlatpakPermissionModel model

    title: i18n("Set Environment Variable")
    standardButtons: QQC2.Dialog.Ok | QQC2.Dialog.Discard
    closePolicy: QQC2.Popup.CloseOnEscape

    QQC2.Overlay.modal: KcmPopupModal {}

    LingmoUI.FormLayout {
        QQC2.TextField {
            id: nameField

            Layout.fillWidth: true

            Keys.onEnterPressed: valueField.forceActiveFocus(Qt.TabFocusReason)
            Keys.onReturnPressed: valueField.forceActiveFocus(Qt.TabFocusReason)
            KeyNavigation.down: valueField

            LingmoUI.FormData.label: i18nc("@label:textbox name of environment variable", "Name:")
        }

        QQC2.TextField {
            id: valueField

            Layout.fillWidth: true

            Keys.onEnterPressed: root.accepted()
            Keys.onReturnPressed: root.accepted()

            LingmoUI.FormData.label: i18nc("@label:textbox value of environment variable", "Value:")
            // No validation needed, empty value is also acceptable.
        }
    }

    function acceptableInput() {
        const name = nameField.text;

        if (permsModel.permissionExists(FlatpakPermissionsSectionType.Environment, name)) {
            return false;
        }

        return permsModel.isEnvironmentVariableNameValid(name);
    }

    onOpened: {
        const ok = standardButton(QQC2.Dialog.Ok);
        ok.enabled = Qt.binding(() => acceptableInput());
        ok.KeyNavigation.up = valueField;

        const discard = standardButton(QQC2.Dialog.Discard);
        discard.KeyNavigation.up = valueField;

        nameField.forceActiveFocus(Qt.PopupFocusReason);
    }

    onAccepted: {
        if (acceptableInput()) {
            const name = nameField.text;
            const value = valueField.text;
            model.addUserEnteredPermission(FlatpakPermissionsSectionType.Environment, name, value);
            close();
        }
    }

    onDiscarded: {
        close();
    }
}
