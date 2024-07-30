// -*- coding: iso-8859-1 -*-
/*
 *   Copyright 2015 Weng Xuetian <wengxt@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2 or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import QtQuick
import QtQuick.Layouts
import org.kde.plasma.plasmoid
import org.kde.plasma.components as PC3
import org.kde.ksvg as KSvg
import org.kde.kirigami as Kirigami

Item {
    function defaultValue(value, d) {
        return (typeof value == 'undefined') ? d : value;
    }

    function deviceLabel() {
        if (dataSource.data["wacomtablet"]["serviceAvailable"]) {
            if (dataModel.count == 0) {
                return i18n("Graphic Tablet - Device not detected.");
            } else {
                if (tabletComboBox.currentIndex >= 0) {
                    return dataModel.get(tabletComboBox.currentIndex).name;
                }
                return "";
            }
        }

        return i18n("Error - Tablet service not available.");
    }
    
    Row {
        id: title
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }
        spacing: Kirigami.Units.smallSpacing
        
        Kirigami.Icon {
            id: titleIcon
            source: "input-tablet"
            width: Kirigami.Units.iconSizes.medium
            height: width
        }
        PC3.Label {
            id: deviceNameLabel
            anchors.verticalCenter: parent.verticalCenter
            width: parent.width - titleIcon.width - parent.spacing
            text: deviceLabel()
            wrapMode: Text.Wrap
        }
    }

    KSvg.SvgItem {
        id: separator
        anchors {
            top: title.bottom
            left: parent.left
            right: parent.right
        }
        elementId: "horizontal-line"
        svg: lineSvg
        height: lineSvg.elementSize("horizontal-line").height
    }

    Row {
        anchors {
            top: separator.bottom
            bottom: parent.bottom
            left: parent.left
            right: parent.right
            topMargin: Kirigami.Units.smallSpacing
            leftMargin: Kirigami.Units.smallSpacing
        }
        visible: !root.active
        spacing: Kirigami.Units.smallSpacing * 2
        
        Kirigami.Icon {
            id: errorIcon
            width: Kirigami.Units.iconSizes.medium
            height: width
            source: "dialog-warning"
        }
        PC3.Label {
            id: errorLabel
            width: parent.width - errorIcon.width - parent.spacing
            text: dataSource.data["wacomtablet"]["serviceAvailable"] ?
                i18n("This widget is inactive because your tablet device is not connected or currently not supported.") :
                i18n("Please start the KDE wacom tablet service.\nThe service is required for tablet detection and profile support.")
            wrapMode: Text.Wrap
        }
    }

    function setProfile() {
        profileComboBox.currentIndex = dataModel.get(tabletComboBox.currentIndex).currentProfile;
    }

    GridLayout {
        id: selector
        anchors {
            left: parent.left
            right: parent.right
            top: separator.bottom
            topMargin: Kirigami.Units.smallSpacing
        }
        visible: root.active
        columns: 2
        PC3.Label {
            text: i18n("Select Tablet:")
        }

        PC3.ComboBox {
            id: tabletComboBox
            Layout.fillWidth: true
            model: dataModel
            textRole: "name"
            onCurrentIndexChanged: {
                profileModel.clear();
                profileComboBox.currentIndex = -1;
                if (tabletComboBox.currentIndex < 0) {
                    return;
                }

                var profiles = dataModel.get(tabletComboBox.currentIndex).profiles;
                if (typeof profiles == "undefined") {
                    return;
                }
                for (var i = 0; i < profiles.length; i++) {
                    profileModel.append({"name" : profiles[i]});
                }

                setProfile();
            }
        }

        PC3.Label {
            text: i18n("Select Profile:")
        }

        ListModel {
            id: profileModel
        }

        PC3.ComboBox {
            id: profileComboBox
            Layout.fillWidth: true
            model: profileModel
            textRole: "name"

            onActivated: {
                if (tabletComboBox.currentIndex < 0) {
                    return;
                }
                var service = dataSource.serviceForSource("wacomtablet");
                var operation = service.operationDescription("SetProfile");
                operation.tabletId = dataModel.get(tabletComboBox.currentIndex).id;
                operation.profile = profileModel.get(index).name;
                service.startOperationCall(operation);
            }
        }
    }

    Connections {
        target: dataSource
        function onConnectedSourcesChanged() {
            tabletComboBox.currentIndex = -1;
            if (dataModel.count > 0) {
                tabletComboBox.currentIndex = 0;
            }
        }

        function onNewData() {
            if (tabletComboBox.currentIndex < 0) {
                return;
            }

            var current = dataModel.get(tabletComboBox.currentIndex)
            if (sourceName == current.DataEngineSource) {
                setProfile();
            }
        }
    }

    PC3.GroupBox {
        visible: root.active
        anchors {
            left: parent.left
            right: parent.right
            top: selector.bottom
            bottom: parent.bottom
            topMargin: Kirigami.Units.smallSpacing
        }
        title: i18nc( "Groupbox Settings for the applet to change some values on the fly", "Settings" )
        GridLayout {
            columns: 2
            PC3.Label {
                visible: defaultValue(dataModel.get(tabletComboBox.currentIndex).hasTouch, false)
                text: i18nc( "Toggle between touch on/off", "Touch:" )
            }

            PC3.CheckBox {
                visible: defaultValue(dataModel.get(tabletComboBox.currentIndex).hasTouch, false);
                checked: defaultValue(dataModel.get(tabletComboBox.currentIndex).touch, false);
            }


            PC3.Label {
                text: i18nc( "Rotation of the tablet pad", "Rotation:" )
            }

            RowLayout {
                RotationButton {
                    rotation: "none"
                    icon.name: "input-tablet"
                    PC3.ToolTip.text: i18nc("Either no orientation or the current screen orientation is applied to the tablet.", "Default Orientation");
                    PC3.ToolTip.delay: Kirigami.Units.toolTipDelay
                    PC3.ToolTip.visible: hovered
                }
                RotationButton {
                    rotation: "cw"
                    icon.name: "object-rotate-left"
                    PC3.ToolTip.text: i18nc("The tablet will be rotated clockwise.", "Rotate Tablet Clockwise")
                    PC3.ToolTip.delay: Kirigami.Units.toolTipDelay
                    PC3.ToolTip.visible: hovered
                }
                RotationButton {
                    rotation: "ccw"
                    icon.name: "object-rotate-right"
                    PC3.ToolTip.text: i18nc("The tablet will be rotated counterclockwise.", "Rotate Tablet Counterclockwise")
                    PC3.ToolTip.delay: Kirigami.Units.toolTipDelay
                    PC3.ToolTip.visible: hovered
                }
                RotationButton {
                    rotation: "half"
                    icon.name: "object-flip-vertical"
                    PC3.ToolTip.text: i18nc("The tablet will be rotated up side down.", "Rotate Tablet Upside-Down")
                    PC3.ToolTip.delay: Kirigami.Units.toolTipDelay
                    PC3.ToolTip.visible: hovered
                }
            }
            PC3.Label {
                text: i18nc( "Toggle between absolute/relative penmode", "Mode:" )
            }
            RowLayout {
                PC3.RadioButton {
                    text: i18nc( "absolute pen movement (pen mode)", "Absolute" )
                    checked: defaultValue(dataModel.get(tabletComboBox.currentIndex).stylusMode, true);
                    onClicked : {
                        if (tabletComboBox.currentIndex < 0) {
                            return;
                        }
                        var service = dataSource.serviceForSource("wacomtablet");
                        var operation = service.operationDescription("SetStylusMode");
                        operation.tabletId = dataModel.get(tabletComboBox.currentIndex).id;
                        operation.mode = "absolute";
                        service.startOperationCall(operation);
                    }
                }
                PC3.RadioButton {
                    text: i18nc( "relative pen movement (mouse mode)", "Relative" )
                    onClicked : {
                        if (tabletComboBox.currentIndex < 0) {
                            return;
                        }
                        var service = dataSource.serviceForSource("wacomtablet");
                        var operation = service.operationDescription("SetStylusMode");
                        operation.tabletId = dataModel.get(tabletComboBox.currentIndex).id;
                        operation.mode = "relative";
                        service.startOperationCall(operation);
                    }
                }
            }
        }
    }

}
