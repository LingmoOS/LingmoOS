// SPDX-FileCopyright: 2014 David Edmundson <kde@davidedmundson.co.uk
// SPDX-FileCopyright: 2020 David Redondo <kde@david-redondo.de
// SPDX-FileCopyright: 2022 Aleix Pol <aleixpol@kde.org>
// SPDX-FileCopyright: 2024 ivan tkachenko <me@ratijas.tk>
// SPDX-License-Identifier: LGPL-2.1-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.private.kquickcontrols as KQuickControlsPrivate

RowLayout {
    id: root

    property bool showClearButton: true
    property bool showCancelButton: false /// TODO KF6 default to true
    property alias modifierOnlyAllowed: helper.modifierOnlyAllowed
    property alias modifierlessAllowed: helper.modifierlessAllowed
    property alias multiKeyShortcutsAllowed: helper.multiKeyShortcutsAllowed
    property alias keySequence: helper.currentKeySequence

    /**
     * This property controls which types of shortcuts are checked for conflicts when the keySequence
     * is set. If a conflict is detected, a messagebox will be shown asking the user to confirm their
     * input. Valid values are combinations of the following flags:
     *   - @p ShortcutType.None Do not check for conflicts.
     *   - @p ShortcutType.StandardShortcuts Check against standard shortcuts. @see KStandardshortcut
     *   - @p ShortcutType.GlobalShortcuts Check against global shortcuts. @see KGlobalAccel
     *
     * The default is `ShortcutType.GlobalShortcuts | ShortcutType.StandardShortcut`
     * @since 5.74
     */
    property alias checkForConflictsAgainst: helper.checkAgainstShortcutTypes

    /**
     * This signal is emitted after the user introduces a new key sequence
     *
     * @since 5.68
     * @deprecated Use keySequenceModified()
     */
    signal captureFinished()

    /***
     * Emitted whenever the key sequence is modified by the user, interacting with the component
     *
     * Either by interacting capturing a key sequence or pressing the clear button.
     *
     * @since 5.99
     */
    signal keySequenceModified()

    /**
     * Start capturing a key sequence. This equivalent to the user clicking on the main button of the item
     * @since 5.70
     */
    function startCapturing() {
        mainButton.checked = true;
    }

    // A layout like RowLayout would automatically and implicitly fillHeight
    // if placed inside a ColumnLayout, so an explicit binding should prevent
    // that behavior. On the contrary, filling width wouldn't hurt, although
    // it doesn't make much sense, as this component is not really adaptive.
    Layout.fillHeight: false

    KQuickControlsPrivate.KeySequenceHelper {
        id: helper

        onGotKeySequence: keySequence => {
            validator.validateSequence(keySequence)
        }

        onQuestionDialogAccepted: validator.accept()
        onQuestionDialogRejected: validator.reject()
    }

    KQuickControlsPrivate.KeySequenceValidator {
        id: validator

        validateTypes: helper.checkAgainstShortcutTypes

        onError: (title, message) => {
            helper.showErrorDialog(title, message)
        }

        onQuestion: (title, message) => {
            helper.showQuestionDialog(title, message)
        }

        onFinished: keySequence => {
            helper.updateKeySequence(keySequence)
            mainButton.checked = false
            root.captureFinished()
            root.keySequenceModified()
        }
    }

    KQuickControlsPrivate.TranslationContext {
        id: _tr
        domain: "kdeclarative6"
    }

    QQC2.Button {
        id: mainButton

        Layout.fillHeight: true

        icon.name: "configure"

        checkable: true
        focus: checked

        hoverEnabled: true

        text: {
            const keySequence = helper.currentKeySequence;
            const text = helper.keySequenceIsEmpty(keySequence)
                ? (helper.isRecording
                    ? _tr.i18nc("What the user inputs now will be taken as the new shortcut", "Input")
                    : _tr.i18nc("No shortcut defined", "None"))
                // Single ampersand gets interpreted by the button as a mnemonic
                // and removed; replace it with a double ampersand so that it
                // will be displayed by the button as a single ampersand, or
                // else shortcuts with the actual ampersand character will
                // appear to be partially empty.
                : helper.keySequenceNativeText(keySequence).replace('&', '&&');
            // These spaces are intentional
            return " " + text + (helper.isRecording ? " ... " : " ");
        }

        Accessible.description: _tr.i18n("Click on the button, then enter the shortcut like you would in the program.\nExample for Ctrl+A: hold the Ctrl key and press A.")
        Accessible.role: Accessible.Button

        QQC2.ToolTip {
            visible: mainButton.hovered
            text: mainButton.Accessible.description
        }

        onCheckedChanged: {
            if (checked) {
                validator.currentKeySequence = root.keySequence
                helper.window = helper.renderWindow(parent.Window.window)
                mainButton.forceActiveFocus()
                helper.startRecording()
            } else if (helper.isRecording) {
                helper.cancelRecording()
            }
        }

        onFocusChanged: {
            if (!focus) {
                mainButton.checked = false
            }
        }
    }

    QQC2.Button {
        id: clearButton
        Layout.fillHeight: true
        Layout.preferredWidth: height
        visible: root.showClearButton && !helper.isRecording
        onClicked: {
            root.keySequence = "";
            root.keySequenceModified();
            root.captureFinished(); // Not really capturing, but otherwise we cannot track this state, hence apps should use keySequenceModified
        }

        enabled: !helper.keySequenceIsEmpty(helper.currentKeySequence)

        hoverEnabled: true
        // icon name determines the direction of the arrow, NOT the direction of the app layout
        icon.name: Qt.application.layoutDirection === Qt.LeftToRight ? "edit-clear-locationbar-rtl" : "edit-clear-locationbar-ltr"

        Accessible.name: _tr.i18nc("@info:tooltip", "Clear Key Sequence")

        QQC2.ToolTip {
            visible: clearButton.hovered
            text: clearButton.Accessible.name
        }
    }

    QQC2.Button {
        Layout.fillHeight: true
        Layout.preferredWidth: height
        onClicked: helper.cancelRecording()
        visible: root.showCancelButton && helper.isRecording

        icon.name: "dialog-cancel"

        Accessible.name: _tr.i18nc("@info:tooltip", "Cancel Key Sequence Recording")

        QQC2.ToolTip {
            visible: parent.hovered
            text: parent.Accessible.name
        }
    }
}
