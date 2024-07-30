/*
    SPDX-FileCopyrightText: 2014 Aleix Pol Gonzalez <aleixpol@blue-systems.com>
    SPDX-FileCopyrightText: 2024 ivan tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.purpose as Purpose

QQC2.ApplicationWindow {
    id: window

    required property var inputData

    visible: true

    minimumWidth: 300
    minimumHeight: 200

    Purpose.AlternativesView {
        id: view

        anchors.fill: parent

        header: Kirigami.Heading {
            text: {
                const urls = window.inputData?.urls ?? [];
                return i18n("Sharing %1", urls.join(", "));
            }
            level: 1
            wrapMode: Text.Wrap
            width: parent?.width
            padding: Kirigami.Units.largeSpacing
        }

        pluginType: "Export"
        inputData: window.inputData

        onFinished: (output, error, message) => {
            if (error !== 0) {
                console.log("Job finished with error", error, message)
            } else if (output.url) {
                console.log("Job finished:", output.url)
            } else {
                console.log("Job finished")
            }
        }
    }
}
