/*
 *   SPDX-FileCopyrightText: 2016 Marco Martin <mart@kde.org>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick 2.3
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.15
import org.kde.kirigami 2.3 as Kirigami

TextField {
    id: field
    property string key
    property string label
    Kirigami.FormData.label: label
    text: lnfLogic[key]
    Layout.minimumWidth: Kirigami.Units.gridUnit * 15
    onAccepted: {
        lnfLogic.name = text;
        timer.running = false;
    }
    onTextChanged: timer.restart()
    Timer {
        id: timer
        interval: 1000
        onTriggered: lnfLogic[key] = field.text
    }
}
