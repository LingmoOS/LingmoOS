/*
 *  SPDX-FileCopyrightText: 2020 Nicolas Fella <nicolas.fella@gmx.de>
 *  SPDX-FileCopyrightText: 2023 Jakob Petsovits <jpetso@petsovits.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtCore
import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Dialogs
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

FocusScope {
    id: root
    required property string command

    implicitHeight: row.implicitHeight
    implicitWidth: row.implicitWidth

    RowLayout {
        id: row
        anchors.fill: parent
        spacing: Kirigami.Units.smallSpacing

        Kirigami.ActionTextField {
            id: commandText
            focus: true
            Layout.fillWidth: true

            text: root.command
            placeholderText: i18n("Enter command or select file…")

            onEditingFinished: { root.command = text; }

            rightActions: Kirigami.Action {
                icon.name: "edit-clear-symbolic"
                visible: commandText.text !== ""
                onTriggered: {
                    commandText.clear();
                    commandText.accepted();
                    commandText.editingFinished();
                }
            }
        }

        QQC2.Button {
            id: selectFileButton
            icon.name: "document-open"
            text: i18n("Select executable file…")
            display: QQC2.AbstractButton.IconOnly
            implicitWidth: implicitHeight
            onClicked: { fileDialogComponent.incubateObject(root); }

            QQC2.ToolTip.visible: hovered
            QQC2.ToolTip.text: text
            QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
        }

        Component {
            id: fileDialogComponent

            FileDialog {
                id: fileDialog
                title: i18n("Select executable file")

                currentFolder: StandardPaths.standardLocations(StandardPaths.HomeLocation)[0]

                onAccepted: {
                    root.command = urlToCommand(selectedFile);
                    destroy();
                }
                onRejected: {
                    destroy();
                }

                function urlToCommand(qmlUrl) {
                    const url = new URL(qmlUrl);
                    let path = decodeURIComponent(url.pathname);
                    // Remove the leading slash from e.g. file:///c:/blah.exe
                    if (url.protocol === "file:" && path.charAt(1) === ':') {
                        path = path.substring(1);
                    }
                    return path.includes(" ") ? ('"' + path.replace('"', '\\"') + '"') : path;
                }

                Component.onCompleted: {
                    if (root.commandExecutable.toString() != "") {
                        const executableUrl = new URL(root.commandExecutable);
                        const dirname = executableUrl.pathname.substring(0, executableUrl.pathname.lastIndexOf("/"));

                        fileDialog.selectedFile = executableUrl;
                        fileDialog.currentFolder = "file://" + (dirname ? dirname : "/");
                    }
                    open();
                }
            }
        }
    }

    readonly property url commandExecutable: {
        // Split executable from parameters, respecting quotes. Example commands:
        // * /bin/bash -i
        // * "/usr/bin/two part.exe" -x
        // * /usr/bin/"two part.exe" -x
        // * /usr/bin/two\ part.exe -x
        const match = command.match(/([^"\s\\]|\\.|"(?:[^"\\]|\\.)*")+/);
        if (!match) {
            return "";
        }
        // Normalize the executable path to unescape spaces and remove quotes.
        const unquoted = match[0].replace(/([^"\s\\])|(\\.)|("(?:[^"\\]|\\.)*")/g, function(any_match, p1, p2, p3) {
            if (p2 === '\\ ') {
                return ' ';
            }
            if (p3 && p3[0] === '"') {
                return p3.substring(1, p3.length - 1);
            }
            return any_match;
        });
        const executableUrl = StandardPaths.findExecutable(unquoted);

        if (executableUrl.toString() !== "") {
            return executableUrl;
        } else if (unquoted[0] === "/") {
            return new URL("file://" + unquoted); // e.g. "/home/my.txt" => "file:///home/my.txt"
        } else if (unquoted[1] === ":") {
            return new URL("file:///" + unquoted); // e.g. "c:/my.txt" => "file:///c:/my.txt"
        }
        return "";
    }
}
