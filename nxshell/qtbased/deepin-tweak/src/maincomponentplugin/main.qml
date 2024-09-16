// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.

// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.11
import org.deepin.dtk 1.0
import QtQuick.Layouts 1.15

AppLoader {
    Component {
        Loader {
            anchors.fill: parent
            source: 'qml/MainPage.qml'
        }
    }
}
