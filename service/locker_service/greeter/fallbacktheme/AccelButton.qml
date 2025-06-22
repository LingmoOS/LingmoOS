/*
SPDX-FileCopyrightText: 2011 Aaron Seigo <aseigo@kde.org>

SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

PlasmaComponents.Button {
    property string label
    property string normalLabel
    property string accelLabel
    property int accelKey: -1

    text: parent.showAccel ? accelLabel : normalLabel

    onLabelChanged: {
        const i = label.indexOf('&');
        if (i < 0) {
            accelLabel = '<u>' + label[0] + '</u>' + label.substring(1, label.length);
            accelKey = label[0].toUpperCase().charCodeAt(0);
            normalLabel = label
        } else {
            const stringToReplace = label.substr(i, 2);
            accelKey = stringToReplace.toUpperCase().charCodeAt(1);
            accelLabel = label.replace(stringToReplace, '<u>' + stringToReplace[1] + '</u>');
            normalLabel = label.replace(stringToReplace, stringToReplace[1]);
        }
    }
}

