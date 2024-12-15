// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
pragma Singleton

QtObject {
    id: root

    property QtObject contentItem: QtObject {
        property int topMargin: 4
        property int bottomMargin: 8
        property int miniHeight: 40
    }
}
