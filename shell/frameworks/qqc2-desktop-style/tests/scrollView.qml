/*
    SPDX-FileCopyrightText: 2018 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Controls

ScrollView {
    height: 100
    width: 300

    ListView {
        model: 100
        delegate: Text {
            text: modelData
        }
    }
}
