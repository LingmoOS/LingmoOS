// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.11
import QtQuick.Window 2.11
import QtQuick.Layouts 1.11
import QtQuick.Templates 2.4 as T
import org.deepin.dtk.impl 1.0 as D
import org.deepin.dtk.style 1.0 as DS

T.Menu {
    id: control

    property bool closeOnInactive: true
    property int maxVisibleItems : DS.Style.arrowListView.maxVisibleItems
    property D.Palette backgroundColor: DS.Style.menu.background
    property var model: control.contentModel
    property Component header
    property Component footer
    readonly property bool existsChecked: {
        for (var i = 0; i < count; ++i) {
            var item = itemAt(i)
            if (item && item.checked)
                return true
        }
        return false
    }
    readonly property bool active: parent && parent.Window.active

    implicitHeight: DS.Style.control.implicitHeight(control)
    implicitWidth: DS.Style.control.implicitWidth(control)
    margins: DS.Style.menu.margins
    overlap: DS.Style.menu.overlap
    padding: DS.Style.menu.padding

    delegate: MenuItem { }

    D.PopupHandle.delegate: PopupWindow {
        blurControl: control
    }

    contentItem: Control {
        contentItem:  ColumnLayout {
            id: viewLayout

            Loader {
                Layout.fillWidth: true
                Layout.preferredHeight: height
                sourceComponent: control.header
            }
            ArrowListView {
                id: contentView
                property int count: contentView.view.count

                view.model: control.model
                Layout.fillWidth: true
                Layout.fillHeight: true
                view.currentIndex: control.currentIndex
                maxVisibleItems: control.maxVisibleItems
                itemHeight: DS.Style.menu.item.height

                function refreshContentItemWidth()
                {
                    for (var i = 0; i < view.count; ++i) {
                        var item = view.model.get(i)
                        if (item) {
                            item.width = view.width
                        }
                    }
                }

                onCountChanged: refreshContentItemWidth()
                onWidthChanged: refreshContentItemWidth()
            }
            Loader {
                Layout.fillWidth: true
                Layout.preferredHeight: height
                sourceComponent: control.footer
            }
        }
    }

    background: Loader {
        active: !control.D.PopupHandle.window
        sourceComponent: FloatingPanel {
            implicitWidth: DS.Style.menu.item.width
            implicitHeight: DS.Style.menu.item.height
            radius: DS.Style.menu.radius
            backgroundColor: control.backgroundColor
        }
    }

    onActiveChanged: {
        if (!active && closeOnInactive) {
            control.close()
        }
    }
}
