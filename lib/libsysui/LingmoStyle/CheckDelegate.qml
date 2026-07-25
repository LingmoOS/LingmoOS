/*
 * SPDX-FileCopyrightText: 2024 Elysia <elysia@lingmo.org>
 *
 * SPDX-License-Identifier: GPL-3.0
 */

import QtQuick
import QtQuick.Templates as T
import QtQuick.Controls.impl
import LingmoUI

T.CheckDelegate {

    property bool disabled: false
    property string contentDescription: ""
    property color borderNormalColor: LingmoTheme.dark ? Qt.rgba(160/255,160/255,160/255,1) : Qt.rgba(136/255,136/255,136/255,1)
    property color bordercheckedColor: LingmoTheme.primaryColor
    property color borderHoverColor: LingmoTheme.dark ? Qt.rgba(167/255,167/255,167/255,1) : Qt.rgba(135/255,135/255,135/255,1)
    property color borderDisableColor: LingmoTheme.dark ? Qt.rgba(82/255,82/255,82/255,1) : Qt.rgba(199/255,199/255,199/255,1)
    property color borderPressedColor: LingmoTheme.dark ? Qt.rgba(90/255,90/255,90/255,1) : Qt.rgba(191/255,191/255,191/255,1)
    property color normalColor: LingmoTheme.dark ? Qt.rgba(45/255,45/255,45/255,1) : Qt.rgba(247/255,247/255,247/255,1)
    property color checkedColor: LingmoTheme.primaryColor
    property color hoverColor: LingmoTheme.dark ? Qt.rgba(72/255,72/255,72/255,1) : Qt.rgba(236/255,236/255,236/255,1)
    property color checkedHoverColor: LingmoTheme.dark ? Qt.darker(checkedColor,1.15) : Qt.lighter(checkedColor,1.15)
    property color checkedPreesedColor: LingmoTheme.dark ? Qt.darker(checkedColor,1.3) : Qt.lighter(checkedColor,1.3)
    property color checkedDisableColor: LingmoTheme.dark ? Qt.rgba(82/255,82/255,82/255,1) : Qt.rgba(199/255,199/255,199/255,1)
    property color disableColor: LingmoTheme.dark ? Qt.rgba(50/255,50/255,50/255,1) : Qt.rgba(253/255,253/255,253/255,1)
    property real size: 18
    property alias textColor: btn_text.textColor
    property bool animationEnabled: LingmoTheme.animationEnabled
    property var clickListener : function(){
        checked = !checked
    }
    property bool indeterminate : (checkState === Qt.PartiallyChecked)

    id: control

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding,
                             implicitIndicatorHeight + topPadding + bottomPadding)

    spacing: 12

    padding: 12
    topPadding: padding - 1
    bottomPadding: padding + 1

    indicator: Rectangle{
        x: control.text ? (control.mirrored ? control.width - width - control.rightPadding : control.leftPadding) : control.leftPadding + (control.availableWidth - width) / 2
        y: control.topPadding + (control.availableHeight - height) / 2
        width: control.size
        height: control.size
        radius: control.size/2
        border.color: {
            if(!enabled){
                return borderDisableColor
            }
            if(checked || (checkState === Qt.PartiallyChecked)){
                return bordercheckedColor
            }
            if(pressed){
                return borderPressedColor
            }
            if(hovered){
                return borderHoverColor
            }
            return borderNormalColor
        }
        border.width: 1
        color: {
            if(checked || (checkState === Qt.PartiallyChecked)){
                if(!enabled){
                    return checkedDisableColor
                }
                if(pressed){
                    return checkedPreesedColor
                }
                if(hovered){
                    return checkedHoverColor
                }
                return checkedColor
            }
            if(!enabled){
                return disableColor
            }
            if(hovered){
                return hoverColor
            }
            return normalColor
        }
        LingmoIcon {
            anchors.centerIn: parent
            iconSource: LingmoIcons.CheckboxIndeterminate
            iconSize: 14
            visible: (checkState === Qt.PartiallyChecked)
            iconColor: LingmoTheme.dark ? Qt.rgba(0,0,0,1) : Qt.rgba(1,1,1,1)
            Behavior on visible {
                enabled: control.animationEnabled
                NumberAnimation{
                    duration: 83
                }
            }
        }

        LingmoIcon {
            anchors.centerIn: parent
            iconSource: LingmoIcons.AcceptMedium
            iconSize: 14
            visible: checked && !(checkState === Qt.PartiallyChecked)
            iconColor: LingmoTheme.dark ? Qt.rgba(0,0,0,1) : Qt.rgba(1,1,1,1)
            Behavior on visible {
                enabled: control.animationEnabled
                NumberAnimation{
                    duration: 83
                }
            }
        }
    }

    contentItem: LingmoText{
        id:btn_text
        leftPadding: control.indicator && !control.mirrored ? control.indicator.width + control.spacing : 0
        rightPadding: control.indicator && control.mirrored ? control.indicator.width + control.spacing : 0
        text: control.text
        elide: Text.ElideRight
        verticalAlignment: Text.AlignVCenter
        visible: text !== ""
        font: control.font
        opacity: enabled ? 1.0 : 0.2
    }
}
