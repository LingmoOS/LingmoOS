// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.11
import QtQuick.Templates as T
import org.deepin.dtk.style 1.0 as DS

T.Control {
    id: control

    implicitWidth: DS.Style.control.implicitWidth(control)
    implicitHeight: DS.Style.control.implicitHeight(control)
}
