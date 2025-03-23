// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.11
import QtQuick.Layouts 1.11
import QtQuick.Templates as T
import org.deepin.dtk 1.0 as D
import org.deepin.dtk.style 1.0 as DS
import org.deepin.dtk.private 1.0 as P

T.ComboBox {
    id: control

    property string iconNameRole
    property string alertText
    property int alertDuration
    property bool showAlert
    property int maxVisibleItems : DS.Style.comboBox.maxVisibleItems
    property D.Palette separatorColor: DS.Style.comboBox.edit.separator
    property var horizontalAlignment: control.flat ? Text.AlignRight : Text.AlignLeft

    implicitWidth: DS.Style.control.implicitWidth(control)
    implicitHeight: DS.Style.control.implicitHeight(control)
    baselineOffset: contentItem.y + contentItem.baselineOffset

    padding: DS.Style.comboBox.padding
    leftPadding: padding + (!control.mirrored || !indicator || !indicator.visible ? 0 : indicator.width + spacing)
    rightPadding: padding + (control.mirrored || !indicator || !indicator.visible ? 0 : indicator.width + spacing)

    delegate: MenuItem {
        useIndicatorPadding: true
        width: control.width
        text: control.textRole ? (Array.isArray(control.model) ? modelData[control.textRole] : (model[control.textRole] === undefined ? modelData[control.textRole] : model[control.textRole])) : modelData
        icon.name: (control.iconNameRole && model[control.iconNameRole] !== undefined) ? model[control.iconNameRole] : null
        highlighted: control.highlightedIndex === index
        hoverEnabled: control.hoverEnabled
        autoExclusive: true
        checked: control.currentIndex === index
    }

    indicator: Loader {
        sourceComponent: control.editable ? editableIndicator : normalIndicator
        x: control.mirrored ? control.padding : control.width - width - control.padding
        y: control.topPadding + (control.availableHeight - height) / 2
        Component {
            id: editableIndicator
            RowLayout {
                spacing: DS.Style.comboBox.edit.indicatorSpacing
                Rectangle {
                    width: 1
                    height: control.height
                    color: control.D.ColorSelector.separatorColor
                }

                D.DciIcon {
                    sourceSize {
                        width: DS.Style.comboBox.edit.indicatorSize
                        height: DS.Style.comboBox.edit.indicatorSize
                    }
                    palette: D.DTK.makeIconPalette(control.palette)
                    name: "entry_option_arrow_down"
                    mode: control.D.ColorSelector.controlState
                    theme: control.D.ColorSelector.controlTheme
                    fallbackToQIcon: false
                }
            }
        }

        Component {
            id: normalIndicator
            D.DciIcon {
                sourceSize {
                    width: DS.Style.comboBox.iconSize
                    height: DS.Style.comboBox.iconSize
                }
                palette: D.DTK.makeIconPalette(control.palette)
                name: "arrow_ordinary_down"
                mode: control.D.ColorSelector.controlState
                theme: control.D.ColorSelector.controlTheme
                fallbackToQIcon: false
            }
        }
    }

    contentItem: RowLayout {
        spacing: DS.Style.comboBox.spacing

        Loader {
            property string iconName: (control.iconNameRole && model.get(control.currentIndex)[control.iconNameRole] !== undefined)
                                      ? model.get(control.currentIndex)[control.iconNameRole] : null
            active: iconName

            sourceComponent: D.DciIcon {
                palette: D.DTK.makeIconPalette(control.palette)
                mode: control.D.ColorSelector.controlState
                theme: control.D.ColorSelector.controlTheme
                name: iconName
                sourceSize: Qt.size(DS.Style.comboBox.iconSize, DS.Style.comboBox.iconSize)
                fallbackToQIcon: true
            }
        }

        T.TextField {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.rightMargin: DS.Style.comboBox.spacing
            text: control.editable ? control.editText : control.displayText

            enabled: control.editable
            autoScroll: control.editable
            readOnly: control.down
            inputMethodHints: control.inputMethodHints
            validator: control.validator
            selectByMouse: true

            color: control.editable ? control.palette.text : control.palette.buttonText
            selectionColor: control.palette.highlight
            selectedTextColor: control.palette.highlightedText
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: control.horizontalAlignment
        }
    }

    background: Item {
        implicitWidth: DS.Style.comboBox.width
        implicitHeight: DS.Style.comboBox.height
        Loader {
            anchors.fill: parent
            active: !control.flat
            sourceComponent: control.editable ? editableComponent : floatingComponent
            property alias comboBox: control
            Component {
                id: editableComponent
                EditPanel {
                    control: comboBox
                    alertText: comboBox.alertText
                    alertDuration: comboBox.alertDuration
                    showAlert: comboBox.showAlert
                }
            }

            Component {
                id: floatingComponent
                P.ButtonPanel {
                    button: comboBox
                }
            }
        }
    }

    popup: Popup {
        implicitWidth: control.width
        contentItem: ArrowListView {
            clip: true
            maxVisibleItems: control.maxVisibleItems
            view.model: control.delegateModel
            view.currentIndex: control.highlightedIndex
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
