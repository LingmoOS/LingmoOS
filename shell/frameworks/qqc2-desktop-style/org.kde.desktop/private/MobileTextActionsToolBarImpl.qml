/*
    SPDX-FileCopyrightText: 2018 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Templates as T
import org.kde.desktop as QQC2
import org.kde.kirigami as Kirigami

QQC2.Popup {
    id: root

    required property /*TextInput | TextEdit*/ Item controlRoot

    Kirigami.OverlayZStacking.layer: Kirigami.OverlayZStacking.Menu
    z: Kirigami.OverlayZStacking.z

    parent: controlRoot?.Window.window?.contentItem ?? null
    modal: false
    focus: false
    closePolicy: T.Popup.NoAutoClose

    x: {
        if (!parent || !controlRoot) {
            return 0;
        }
        const desiredX = controlRoot.mapToItem(parent, controlRoot.positionToRectangle(controlRoot.selectionStart).x, 0).x - width / 2;
        const maxX = parent.width - width;

        return Math.min(Math.max(0, desiredX), maxX);
    }

    y: {
        if (!parent || !controlRoot) {
            return 0;
        }
        const desiredY = controlRoot.mapToItem(parent, 0, controlRoot.positionToRectangle(controlRoot.selectionStart).y).y - height;
        const maxY = parent.height - height;

        if (desiredY >= 0) {
            return Math.min(maxY, desiredY);
        } else {
            const desiredY = controlRoot.mapToItem(parent, 0, controlRoot.positionToRectangle(controlRoot.selectionEnd).y + Math.round(Kirigami.Units.gridUnit * 1.5)).y;
            return Math.min(maxY, Math.max(0, desiredY));
        }
    }

    width: contentItem.implicitWidth + leftPadding + rightPadding
    visible: true

    contentItem: RowLayout {
        QQC2.ToolButton {
            focusPolicy: Qt.NoFocus
            text: qsTr("Cut")
            display: T.AbstractButton.IconOnly
            icon.name: "edit-cut-symbolic"
            visible: controlRoot.selectedText.length > 0 && (!(controlRoot instanceof TextInput) || controlRoot.echoMode === TextInput.Normal)
            onClicked: {
                controlRoot.cut();
            }
        }
        QQC2.ToolButton {
            focusPolicy: Qt.NoFocus
            text: qsTr("Copy")
            display: T.AbstractButton.IconOnly
            icon.name: "edit-copy-symbolic"
            visible: controlRoot.selectedText.length > 0 && (!(controlRoot instanceof TextInput) || controlRoot.echoMode === TextInput.Normal)
            onClicked: {
                controlRoot.copy();
            }
        }
        QQC2.ToolButton {
            focusPolicy: Qt.NoFocus
            text: qsTr("Paste")
            display: T.AbstractButton.IconOnly
            icon.name: "edit-paste-symbolic"
            visible: controlRoot.canPaste
            onClicked: {
                controlRoot.paste();
            }
        }
    }
}
