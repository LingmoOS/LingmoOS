/*
    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
    SPDX-FileCopyrightText: 2024 David Redondo <kde@david-redondo.de>
*/


import QtQuick
import QtQuick.Controls as QQC2
import org.kde.plasma.workspace.dialogs as PWD

PWD.SystemDialog {

    readonly property string app: ""

    iconName: "dialog-input-devices"
    title: i18nc("@title:window", "Input Capture Requested")
    subtitle: app === "" ? i18nc("The application is unknown", "An application requested to capture input events") : i18nc("%1 is the name of the application", "%1 requested to capture input events", app)

    standardButtons: QQC2.DialogButtonBox.Ok | QQC2.DialogButtonBox.Cancel

    Component.onCompleted:  dialogButtonBox.standardButton(QQC2.DialogButtonBox.Ok).text = i18nc("@action:button", "Allow")
}

