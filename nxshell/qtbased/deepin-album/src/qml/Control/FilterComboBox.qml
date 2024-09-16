// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts
import QtQml
import org.deepin.dtk 1.0
import org.deepin.dtk.style 1.0 as DS

ComboBox {
    id: comboBox
    textRole: "text"
    iconNameRole: "icon"
    flat: true

    model: ListModel {
        ListElement { text: qsTr("All"); icon: "selectall" }
        ListElement { text: qsTr("Photos"); icon: "images" }
        ListElement { text: qsTr("Videos"); icon: "videos" }
    }

    delegate: MenuItem {
        useIndicatorPadding: true
        width: parent.width
        text: comboBox.textRole ? (Array.isArray(comboBox.model) ? modelData[comboBox.textRole] : model[comboBox.textRole]) : modelData
        icon.name: (comboBox.iconNameRole && model[comboBox.iconNameRole] !== undefined) ? model[comboBox.iconNameRole] : null
        highlighted: comboBox.highlightedIndex === index
        hoverEnabled: comboBox.hoverEnabled
        autoExclusive: true
        checked: comboBox.currentIndex === index
    }

    // 为符合UI效果，弹框宽度比下拉框宽度略大
    popup: Popup {
        clip: true
        x: -15
        implicitWidth: comboBox.width + 15
        contentItem: ArrowListView {
            maxVisibleItems: comboBox.maxVisibleItems
            view.model: comboBox.delegateModel
            view.currentIndex: comboBox.highlightedIndex
            view.highlightRangeMode: ListView.ApplyRange
            view.highlightMoveDuration: 0
        }

        background: FloatingPanel {
            implicitWidth: DS.Style.menu.item.width
            implicitHeight: DS.Style.menu.item.height
            radius: DS.Style.menu.radius
            backgroundColor: DS.Style.menu.background
        }
    }
}

