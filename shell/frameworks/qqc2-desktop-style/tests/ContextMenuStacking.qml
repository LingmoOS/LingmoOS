/*
 *  SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import QtQuick.Templates as T
import org.kde.kirigami as Kirigami

QQC2.ApplicationWindow {
    id: root

    width: 400
    height: 400
    visible: true

    Kirigami.SelectableLabel {
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            margins: Kirigami.Units.largeSpacing
            topMargin: Kirigami.Units.gridUnit * 3
        }
        text: `<ol>
            <li>TextField is parented to an item in Popup A.</li>
            <li>Context menu opens for the TextField, inheriting z index of Popup A.</li>
            <li>Then TextField is moved to a Popup B which is stacked much higher (e.g. in Notification layer).</li>
            <li>Context menu is requested again for the same TextField.</li>
            <li>Menu should inherit new z index and be stacked on top of Popup B.</li>
            </ol>`
    }

    QQC2.TextField {
        id: textField
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.margins: Kirigami.Units.largeSpacing
        placeholderText: "Right click me"
    }

    component ControlPanel: Column {
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        anchors.margins: Kirigami.Units.largeSpacing
        spacing: Kirigami.Units.smallSpacing

        QQC2.Button {
            text: "Move TextField to Popup A"
            onClicked: {
                textField.parent = placeholderA;
            }
        }
        QQC2.Button {
            text: "Open Popup A"
            onClicked: {
                popupA.open();
            }
        }
        QQC2.Button {
            text: "Move TextField to Popup B"
            onClicked: {
                textField.parent = placeholderB;
            }
        }
        QQC2.Button {
            text: "Open Popup B"
            onClicked: {
                popupB.open();
            }
        }
    }

    ControlPanel {}

    QQC2.Popup {
        id: popupA

        Kirigami.OverlayZStacking.layer: Kirigami.OverlayZStacking.Dialog
        z: Kirigami.OverlayZStacking.z

        parent: root.QQC2.Overlay.overlay
        x: 0
        y: 0
        width: 300
        height: 300
        margins: Kirigami.Units.gridUnit
        closePolicy: QQC2.Popup.NoAutoClose | QQC2.Popup.CloseOnPressOutside
        modal: false

        contentItem: Item {
            id: placeholderA

            ControlPanel {}
        }
    }

    QQC2.Popup {
        id: popupB

        Kirigami.OverlayZStacking.layer: Kirigami.OverlayZStacking.Notification
        z: Kirigami.OverlayZStacking.z

        parent: root.QQC2.Overlay.overlay
        anchors.centerIn: parent
        width: 300
        height: 300
        margins: Kirigami.Units.gridUnit
        closePolicy: QQC2.Popup.NoAutoClose | QQC2.Popup.CloseOnPressOutside
        modal: false

        contentItem: Item {
            id: placeholderB

            ControlPanel {}
        }
    }
}
