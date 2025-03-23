// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.11
import org.deepin.dtk.style 1.0 as DS
import ".."

Item {
    id: control
    property Item progressBar

    Loader {
        anchors.fill: parent
        sourceComponent: progressBar.formatText ? _textPanelComponent : _normalTextComponent
    }

    Component {
        id: _textPanelComponent
        BoxPanel {}
    }

    Component {
        id: _normalTextComponent
        Rectangle {
            radius: DS.Style.control.radius
            color: Qt.rgba(0, 0, 0, 0.1);
        }
    }
}
