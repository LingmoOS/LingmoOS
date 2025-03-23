// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.11
import QtQuick.Templates as T
import org.deepin.dtk.style 1.0 as DS
import org.deepin.dtk.private 1.0 as P

T.ProgressBar {
    id: control
    property string formatText
    property bool animationStop: false
    implicitWidth: DS.Style.control.implicitWidth(control)
    implicitHeight: DS.Style.control.implicitHeight(control)

    contentItem: P.ProgressBarImpl {
        progressBar: control
        formatText: control.formatText
        animationStop: control.animationStop
    }

    background: P.ProgressBarPanel {
        implicitWidth: DS.Style.progressBar.width
        implicitHeight: DS.Style.progressBar.height
        progressBar: control
    }
}
