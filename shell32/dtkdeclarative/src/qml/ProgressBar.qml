// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.11
import QtQuick.Templates 2.4 as T
import org.deepin.dtk.style 1.0 as DS
import "private"

T.ProgressBar {
    id: control
    property string formatText
    property bool animationStop: false
    implicitWidth: DS.Style.control.implicitWidth(control)
    implicitHeight: DS.Style.control.implicitHeight(control)

    contentItem: ProgressBarImpl {
        progressBar: control
        formatText: control.formatText
        animationStop: control.animationStop
    }

    background: ProgressBarPanel {
        implicitWidth: DS.Style.progressBar.width
        implicitHeight: DS.Style.progressBar.height
        progressBar: control
    }
}
