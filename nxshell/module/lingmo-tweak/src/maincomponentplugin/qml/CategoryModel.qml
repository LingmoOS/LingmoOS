// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.

// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.0
import QtQml.Models 2.15

ListModel {
    ListElement {
        title: qsTr("Welcome")
        icon: "/images/Welcome.png"
        source: "Categories/Welcome.qml"
    }
    ListElement {
        title: qsTr("Other")
        icon: "/images/Other.svg"
        source: "Categories/Other.qml"
    }
}

