// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15

import org.lingmo.ds 1.0
import org.lingmo.dtk 1.0 as D

AppletItem {
    id: control
    implicitWidth: childrenRect.width
    implicitHeight: childrenRect.height
    property int selectIndex: indexByValue(Applet.effectType)
    property int checkedIndex: indexByValue(Applet.effectType)

    enum WindowEffectType {
        Default = 0,
        Best,
        Better,
        Good,
        Normal,
        Compatible
    }

    Connections {
        target: control.Panel
        enabled: match(control.Panel.osdType)
        function onVisibleChanged() {
            if (!control.Panel.visible) {
                Applet.effectType = effectModel.get(selectIndex).value
            }
        }
    }

    function indexByValue(value) {
        for (var i = 0; i < effectModel.count; i++) {
            if (effectModel.get(i).value === value) {
                return i
            }
        }
        return -1
    }

    function update(osdType)
    {
        if (match(osdType)) {
            if (selectIndex === effectModel.count - 1) {
                selectIndex = 0
            } else {
                selectIndex++
            }
            return true
        }
        return false
    }

    function match(osdType)
    {
        return osdType === "SwitchWM"
    }

    ListView {
        id: listview
        model: effectModel
        height: contentHeight
        width: 500
        spacing: 5
        delegate: D.ItemDelegate {
            checkable: true
            checked: control.selectIndex === index
            icon.name: model.icon
            icon.width: 64
            icon.height: 64
            width: listview.width
            rightPadding: 10
            contentFlow: true
            content: RowLayout {
                spacing: 10
                ColumnLayout {
                    D.Label {
                        text: model.title
                        font {
                            family: D.DTK.fontManager.t5.family
                            pointSize: D.DTK.fontManager.t5.pointSize
                            bold: true
                        }
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignLeft
                    }

                    D.Label {
                        text: model.description
                        font: D.DTK.fontManager.t8
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignLeft
                        wrapMode: Text.WordWrap
                    }
                }
                D.DciIcon {
                    visible: control.checkedIndex === index
                    sourceSize: Qt.size(24, 24)
                    name: "item_checked"
                }
            }
        }
    }

    ListModel {
        id: effectModel
        ListElement {
            value: main.WindowEffectType.Normal
            icon: "osd_optimal_performance"
            title: qsTr("Optimal performance")
            description: qsTr("Optimal performance: Close all interface and window effects to ensure efficient system operation")
        }
        ListElement {
            value: main.WindowEffectType.Better
            icon: "osd_balance_effect"
            title: qsTr("Balance")
            description: qsTr("Balance: Limit some window effects to ensure excellent visual experience while maintaining smooth system operation")
        }
        ListElement {
            value: main.WindowEffectType.Best
            icon: "osd_best_visuals"
            title: qsTr("Best Visuals")
            description: qsTr("Best Visual: Enable all interface and window effects to experience the best visual effects")
        }
    }
}
