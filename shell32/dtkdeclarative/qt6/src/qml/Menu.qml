// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.11
import QtQuick.Window 2.11
import QtQuick.Layouts 1.11
import QtQuick.Templates as T
import org.deepin.dtk 1.0 as D
import org.deepin.dtk.style 1.0 as DS

T.Menu {
    id: control

    property bool closeOnInactive: true
    property int maxVisibleItems : DS.Style.arrowListView.maxVisibleItems
    property D.Palette backgroundColor: DS.Style.menu.background
    property D.Palette backgroundNoBlurColor: DS.Style.menu.backgroundNoBlur
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
        topPadding: 15 // TODO how to clip radius
        bottomPadding: topPadding
        leftPadding: 0
        rightPadding: leftPadding

        contentItem:  ColumnLayout {
            id: viewLayout
            spacing: 0

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
                view.highlightFollowsCurrentItem: true
                view.highlightMoveDuration: 50
                view.highlightMoveVelocity: -1

                view.highlight: Rectangle {
                    id: highlightRect
                    anchors.left: parent ? parent.left : undefined
                    anchors.right: parent ? parent.right : undefined
                    anchors.leftMargin: 0
                    anchors.rightMargin: 0
                    scale: D.DTK.hasAnimation ? 0.9 : 1.0
                    property D.Palette backgroundColor: DS.Style.highlightPanel.background
                    property D.Palette submenuOpenedItemHighlightColor: DS.Style.menu.submenuOpenedItemHighlight
                    property D.Palette itemHighlightShadowColor: DS.Style.menu.itemHighlightShadow
                    color: {
                        let item = control.itemAt(control.currentIndex)
                        if (item && item.subMenu) {
                            return D.ColorSelector.submenuOpenedItemHighlightColor
                        } else {
                            return D.ColorSelector.backgroundColor
                        }
                    }
                    radius: 1
                    Component.onCompleted: {
                        scale = 1.0
                    }
                    Behavior on scale {
                        NumberAnimation { duration: 100 }
                    }
                    BoxInsetShadow {
                        visible: highlightRect.color === highlightRect.D.ColorSelector.backgroundColor
                        anchors.fill: parent
                        z: DTK.AboveOrder
                        cornerRadius: parent.radius
                        shadowOffsetX: 0
                        shadowOffsetY: -1
                        shadowBlur: 1
                        spread: 1
                        shadowColor: highlightRect.D.ColorSelector.itemHighlightShadowColor
                    }
                }
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
            blurMultiplier: 8.0
            backgroundColor: control.backgroundColor
            backgroundNoBlurColor: control.backgroundNoBlurColor
            outsideBorderColor: null
        }
    }

    onActiveChanged: {
        if (!active && closeOnInactive) {
            control.close()
        }
    }
}
