// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.11
import QtQuick.Templates as T
import org.deepin.dtk.style 1.0 as DS

T.SpinBox {
    id: control
    // alert control properties
    property alias alertText: panel.alertText
    property alias alertDuration: panel.alertDuration
    property alias showAlert: panel.showAlert

    implicitWidth: {
        var content = control.implicitContentWidth !== undefined
                ? control.implicitContentWidth
                : (control.contentItem ? control.contentItem.implicitWidth : 0)
        content += 2 * control.padding +
                (up.indicator ? up.indicator.implicitWidth : 0) +
                (down.indicator ? down.indicator.implicitWidth : 0)
        return Math.max(DS.Style.control.backgroundImplicitWidth(control), content)
    }
    implicitHeight:  Math.max(DS.Style.control.implicitHeight(control),
                             up.indicator ? up.indicator.implicitHeight : 0,
                             down.indicator ? down.indicator.implicitHeight : 0)
    baselineOffset: contentItem.y + contentItem.baselineOffset
    padding: 0
    rightPadding: padding + ((down.indicator ? (down.indicator.width + DS.Style.spinBox.spacing)
                                             :  (up.indicator ? (up.indicator.width + DS.Style.spinBox.spacing)
                                                              : 0)))

    validator: IntValidator {
        locale: control.locale.name
        bottom: Math.min(control.from, control.to)
        top: Math.max(control.from, control.to)
    }

    background: EditPanel {
        id: panel
        control: control
        implicitWidth: DS.Style.spinBox.width
        implicitHeight: DS.Style.spinBox.height
    }

    contentItem: TextInput {
        text: control.displayText
        font: control.font
        color: control.palette.text
        selectionColor: control.palette.highlight
        selectedTextColor: control.palette.highlightedText
        horizontalAlignment: Qt.AlignLeft
        verticalAlignment: Qt.AlignVCenter
        leftPadding: DS.Style.spinBox.spacing
        readOnly: !control.editable
        validator: control.validator
        inputMethodHints: control.inputMethodHints
        selectByMouse: control.editable
    }

    up.indicator: SpinBoxIndicator {
        anchors {
            right: control.right
            rightMargin: DS.Style.spinBox.indicatorSpacing
            top: control.top
            topMargin: control.height / 2 - (down.indicator ? height : height / 2)
        }

        direction: SpinBoxIndicator.IndicatorDirection.UpIndicator
        singleIndicator: down.indicator
        pressed: up.pressed
        spinBox: control
    }

    down.indicator: SpinBoxIndicator {
        anchors {
            right: control.right
            rightMargin: DS.Style.spinBox.indicatorSpacing
            bottom: control.bottom
            bottomMargin: control.height / 2 - (up.indicator ? height : height / 2)
        }

        direction: SpinBoxIndicator.IndicatorDirection.DownIndicator
        singleIndicator: up.indicator
        pressed: down.pressed
        spinBox: control
    }
}
