/*
    SPDX-FileCopyrightText: 2020 Devin Lin <espidev@gmail.com>
    SPDX-FileCopyrightText: 2021 Carl Schwan <carlschwan@kde.org>
    SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

pragma Singleton

import QtQml.Models
import QtQuick
import QtQuick.Templates as T
import org.kde.desktop as QQC2
import org.kde.kirigami as Kirigami
import org.kde.sonnet as Sonnet

QQC2.Menu {
    id: root

    property Item target
    property bool deselectWhenMenuClosed: true
    property int restoredCursorPosition: 0
    property int restoredSelectionStart
    property int restoredSelectionEnd
    property bool persistentSelectionSetting

    // assuming that Instantiator::active is bound to target.Kirigami.SpellCheck.enabled
    property Instantiator/*<Sonnet.SpellcheckHighlighter>*/ spellcheckHighlighterInstantiator

    // assuming that spellchecker's active state is not writable, use target.Kirigami.SpellCheck.enabled instead.
    readonly property Sonnet.SpellcheckHighlighter spellcheckHighlighter:
        spellcheckHighlighterInstantiator?.object as Sonnet.SpellcheckHighlighter

    property /*list<string>*/var spellcheckSuggestions: []

    Component.onCompleted: persistentSelectionSetting = persistentSelectionSetting // break binding

    property var runOnMenuClose: () => {}

    function storeCursorAndSelection() {
        restoredCursorPosition = target.cursorPosition;
        restoredSelectionStart = target.selectionStart;
        restoredSelectionEnd = target.selectionEnd;
    }

    // target is pressed with mouse
    function targetClick(
        handlerPoint,
        target,
        spellcheckHighlighterInstantiator,
        mousePosition,
    ) {
        if (!(target instanceof TextInput || target instanceof TextEdit)) {
            console.warn("Target not supported by standard context menu:", target);
            return;
        }
        if (handlerPoint.pressedButtons === Qt.RightButton) { // only accept just right click
            if (visible) {
                deselectWhenMenuClosed = false; // don't deselect text if menu closed by right click on textfield
                dismiss();
            } else {
                this.target = target;
                target.persistentSelection = true; // persist selection when menu is opened

                this.spellcheckHighlighterInstantiator = spellcheckHighlighterInstantiator;

                spellcheckSuggestions = (spellcheckHighlighter && mousePosition)
                    ? spellcheckHighlighter.suggestions(mousePosition)
                    : [];

                storeCursorAndSelection();
                popup(target);
                // slightly locate context menu away from mouse so no item is selected when menu is opened
                x += 1
                y += 1
            }
        } else {
            dismiss();
        }
    }

    // context menu keyboard key
    function targetKeyPressed(event, target) {
        if (event.modifiers === Qt.NoModifier && event.key === Qt.Key_Menu) {
            this.target = target;
            target.persistentSelection = true; // persist selection when menu is opened
            storeCursorAndSelection();
            popup(target);
        }
    }

    function __hasSelectedText(): bool {
        return target !== null
            && target.selectedText !== "";
    }

    function __editable(): bool {
        return target !== null
            && !target.readOnly;
    }

    function __hasSpellcheckCapability(): bool {
        return __editable()
            && spellcheckHighlighterInstantiator !== null;
    }

    function __showSpellcheckActions(): bool {
        return __editable()
            && spellcheckHighlighter !== null
            && spellcheckHighlighter.active
            && spellcheckHighlighter.wordIsMisspelled;
    }

    // Show actions which should normally be hidden for password field
    function __showPasswordRestrictedActions(): bool {
        return target !== null
            && target.echoMode !== TextInput.PasswordEchoOnEdit
            && target.echoMode !== TextInput.Password;
    }

    // Show text editing actions which should normally be hidden for password field
    function __showPasswordRestrictedEditingActions(): bool {
        return __showPasswordRestrictedActions() && !target.readOnly;
    }

    modal: true

    // deal with whether text should be deselected
    onClosed: {
        // reset parent, so OverlayZStacking could refresh z order next time
        // this menu is about to open for the same item that might have been
        // reparented to a different popup.
        parent = null;

        // restore text field's original persistent selection setting
        target.persistentSelection = persistentSelectionSetting
        // deselect text field text if menu is closed not because of a right click on the text field
        if (deselectWhenMenuClosed) {
            target.deselect();
        }
        deselectWhenMenuClosed = true;

        // restore cursor position
        target.forceActiveFocus();
        target.cursorPosition = restoredCursorPosition;
        target.select(restoredSelectionStart, restoredSelectionEnd);

        // run action, and free memory
        try {
            runOnMenuClose();
        } catch (e) {
            console.error(e);
            console.trace();
        }
        runOnMenuClose = () => {};

        // clean up spellchecker
        spellcheckHighlighterInstantiator = null;
        spellcheckSuggestions = [];
    }

    onOpened: {
        runOnMenuClose = () => {};
    }

    Instantiator {
        active: root.__showSpellcheckActions()

        model: root.spellcheckSuggestions
        delegate: QQC2.MenuItem {
            required property string modelData

            text: modelData

            onClicked: {
                root.deselectWhenMenuClosed = false;
                root.runOnMenuClose = () => {
                    root.spellcheckHighlighter.replaceWord(modelData);
                };
            }
        }
        onObjectAdded: (index, object) => {
            root.insertItem(0, object);
        }
        onObjectRemoved: (index, object) => {
            root.removeItem(object);
        }
    }

    QQC2.MenuItem {
        visible: root.__showSpellcheckActions() && root.spellcheckSuggestions.length === 0
        action: T.Action {
            enabled: false
            text: root.spellcheckHighlighter
                ? qsTr('No Suggestions for "%1"')
                    .arg(root.spellcheckHighlighter.wordUnderMouse)
                : ""
        }
    }

    QQC2.MenuSeparator {
        visible: root.__showSpellcheckActions()
    }

    QQC2.MenuItem {
        visible: root.__showSpellcheckActions()
        action: T.Action {
            text: root.spellcheckHighlighter
                ? qsTr('Add "%1" to Dictionary')
                    .arg(root.spellcheckHighlighter.wordUnderMouse)
                : ""

            onTriggered: {
                root.deselectWhenMenuClosed = false;
                root.runOnMenuClose = () => {
                    root.spellcheckHighlighter.addWordToDictionary(root.spellcheckHighlighter.wordUnderMouse);
                };
            }
        }
    }

    QQC2.MenuItem {
        visible: root.__showSpellcheckActions()
        action: T.Action {
            text: qsTr("Ignore")
            onTriggered: {
                root.deselectWhenMenuClosed = false;
                root.runOnMenuClose = () => {
                    root.spellcheckHighlighter.ignoreWord(root.spellcheckHighlighter.wordUnderMouse);
                };
            }
        }
    }

    QQC2.MenuItem {
        visible: root.__hasSpellcheckCapability()

        checkable: true
        checked: root.target?.Kirigami.SpellCheck.enabled ?? false
        text: qsTr("Spell Check")

        onToggled: {
            if (root.target) {
                root.target.Kirigami.SpellCheck.enabled = checked;
            }
        }
    }

    QQC2.MenuSeparator {
        visible: root.__hasSpellcheckCapability()
            && (root.__editable() || root.__showPasswordRestrictedActions())
    }

    QQC2.MenuItem {
        action: T.Action {
            icon.name: "edit-undo-symbolic"
            text: qsTr("Undo")
            shortcut: StandardKey.Undo
        }
        visible: root.__showPasswordRestrictedEditingActions()
        enabled: root.target?.canUndo ?? false
        onTriggered: {
            root.deselectWhenMenuClosed = false;
            root.runOnMenuClose = () => {
                root.target.undo();
            };
        }
    }
    QQC2.MenuItem {
        action: T.Action {
            icon.name: "edit-redo-symbolic"
            text: qsTr("Redo")
            shortcut: StandardKey.Redo
        }
        visible: root.__showPasswordRestrictedEditingActions()
        enabled: root.target?.canRedo ?? false
        onTriggered: {
            root.deselectWhenMenuClosed = false;
            root.runOnMenuClose = () => {
                root.target.redo();
            };
        }
    }
    QQC2.MenuSeparator {
        visible: root.__showPasswordRestrictedEditingActions()
    }
    QQC2.MenuItem {
        action: T.Action {
            icon.name: "edit-cut-symbolic"
            text: qsTr("Cut")
            shortcut: StandardKey.Cut
        }
        visible: root.__showPasswordRestrictedEditingActions()
        enabled: root.__hasSelectedText()
        onTriggered: {
            root.deselectWhenMenuClosed = false;
            root.runOnMenuClose = () => {
                root.target.cut();
            };
        }
    }
    QQC2.MenuItem {
        action: T.Action {
            icon.name: "edit-copy-symbolic"
            text: qsTr("Copy")
            shortcut: StandardKey.Copy
        }
        visible: root.__showPasswordRestrictedActions()
        enabled: root.__hasSelectedText()
        onTriggered: {
            root.deselectWhenMenuClosed = false;
            root.runOnMenuClose = () => {
                root.target.copy();
            };
        }
    }
    QQC2.MenuItem {
        action: T.Action {
            icon.name: "edit-paste-symbolic"
            text: qsTr("Paste")
            shortcut: StandardKey.Paste
        }
        visible: root.__editable()
        enabled: target?.canPaste ?? false
        onTriggered: {
            root.deselectWhenMenuClosed = false;
            root.runOnMenuClose = () => {
                root.target.paste();
            };
        }
    }
    QQC2.MenuItem {
        action: T.Action {
            icon.name: "edit-delete-symbolic"
            text: qsTr("Delete")
            shortcut: StandardKey.Delete
        }
        visible: root.__editable()
        enabled: root.__hasSelectedText()
        onTriggered: {
            root.deselectWhenMenuClosed = false;
            root.runOnMenuClose = () => {
                root.target.remove(root.target.selectionStart, root.target.selectionEnd);
            };
        }
    }
    QQC2.MenuSeparator {
        visible: root.target !== null
            && (root.__editable() || root.__showPasswordRestrictedActions())
    }
    QQC2.MenuItem {
        action: T.Action {
            icon.name: "edit-select-all-symbolic"
            text: qsTr("Select All")
            shortcut: StandardKey.SelectAll
        }
        visible: root.target !== null
        onTriggered: {
            root.deselectWhenMenuClosed = false;
            root.runOnMenuClose = () => {
                root.target.selectAll();
            };
        }
    }
}
