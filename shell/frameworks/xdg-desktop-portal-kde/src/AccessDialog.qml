/*
 * SPDX-FileCopyrightText: 2021 Aleix Pol Gonzalez <aleixpol@kde.org>
 */

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.plasma.workspace.dialogs as PWD

PWD.SystemDialog {
    id: root

    property alias body: bodyLabel.text
    property string acceptLabel
    property string rejectLabel

    QQC2.Label {
        id: bodyLabel
        Layout.fillWidth: true
        wrapMode: Text.WordWrap
    }

    standardButtons: QQC2.DialogButtonBox.Ok | QQC2.DialogButtonBox.Cancel

    Component.onCompleted: {
        if (root.acceptLabel.length > 0) {
            dialogButtonBox.button(QQC2.DialogButtonBox.Ok).text = Qt.binding(() => root.acceptLabel);
        }
        if (root.rejectLabel.length > 0) {
            dialogButtonBox.button(QQC2.DialogButtonBox.Cancel).text = Qt.binding(() => root.rejectLabel);
        }
    }
}
