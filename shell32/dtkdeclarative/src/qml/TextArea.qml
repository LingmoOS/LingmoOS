// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.11
import QtQuick.Controls.impl 2.4
import QtQuick.Templates 2.4 as T
import org.deepin.dtk.impl 1.0 as D
import org.deepin.dtk.style 1.0 as DS

T.TextArea {
    id: control

    property D.Palette placeholderTextColor: DS.Style.edit.placeholderText
    implicitWidth: Math.max(DS.Style.control.implicitWidth(control), placeholder.implicitWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(DS.Style.control.implicitHeight(control), placeholder.implicitHeight + topPadding + bottomPadding)

    padding: DS.Style.control.padding

    color: palette.text
    selectionColor: palette.highlight
    selectedTextColor: palette.highlightedText
    onEffectiveHorizontalAlignmentChanged: placeholder.effectiveHorizontalAlignmentChanged()

    Loader {
        id: placeholder
        active: !control.length && !control.preeditText && (!control.activeFocus || control.horizontalAlignment !== Qt.AlignHCenter)
        x: control.leftPadding
        y: control.topPadding
        width: control.width - (control.leftPadding + control.rightPadding)
        height: control.height - (control.topPadding + control.bottomPadding)
        signal effectiveHorizontalAlignmentChanged

        sourceComponent: PlaceholderText {
            text: control.placeholderText
            font: control.font
            color: control.D.ColorSelector.placeholderTextColor
            verticalAlignment: control.verticalAlignment
            elide: Text.ElideRight
            renderType: control.renderType
        }
    }

    background: EditPanel {
        control: control
        implicitWidth: DS.Style.edit.width
        implicitHeight: DS.Style.edit.textAreaHeight
    }
}
