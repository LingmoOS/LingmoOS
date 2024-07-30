// SPDX-FileCopyrightText: 2021-2024 Devin Lin <devin@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import QtQuick.Window
import Qt5Compat.GraphicalEffects
import QtQuick.Templates as T
import org.kde.lingmoui as LingmoUI

Item {
    id: root

    // -- public API: should match lingmo-workspace implementation --

    default property Item mainItem
    property string mainText: ""
    property string subtitle: ""
    property string iconName
    property list<T.Action> actions
    readonly property alias dialogButtonBox: footerButtonBox

    required property Window window
    readonly property real minimumHeight: implicitWidth
    readonly property real minimumWidth: implicitHeight
    readonly property int flags: Qt.FramelessWindowHint | Qt.Dialog
    property var standardButtons // footerButtonBox standardButtons
    readonly property int spacing: LingmoUI.Units.gridUnit

    function present() {
        window.showMaximized();
    }

    onWindowChanged: {
        if (window) {
            window.color = Qt.rgba(0, 0, 0, 0.5);
        }
    }

    Item {
        id: windowItem
        anchors.centerIn: parent
        // margins for shadow
        implicitWidth: Math.min(Screen.width, control.implicitWidth + 2 * LingmoUI.Units.gridUnit)
        implicitHeight: Math.min(Screen.height, control.implicitHeight + 2 * LingmoUI.Units.gridUnit)

        // shadow
        RectangularGlow {
            id: glow
            anchors.topMargin: 1
            anchors.fill: control
            cached: true
            glowRadius: 2
            cornerRadius: LingmoUI.Units.gridUnit
            spread: 0.1
            color: Qt.rgba(0, 0, 0, 0.4)
        }

        // actual window
        QQC2.Control {
            id: control
            anchors.fill: parent
            anchors.margins: glow.cornerRadius
            topPadding: root.spacing
            bottomPadding: root.spacing
            rightPadding: root.spacing
            leftPadding: root.spacing

            implicitWidth: LingmoUI.Units.gridUnit * 22

            background: Item {
                Rectangle { // border
                    anchors.fill: parent
                    anchors.margins: -1
                    radius: LingmoUI.Units.largeSpacing + 1
                    color: Qt.darker(LingmoUI.Theme.backgroundColor, 1.5)
                }
                Rectangle { // background colour
                    anchors.fill: parent
                    radius: LingmoUI.Units.largeSpacing
                    color: LingmoUI.Theme.backgroundColor
                }
            }

            contentItem: ColumnLayout {
                id: column
                spacing: 0

                // header
                LingmoUI.Heading {
                    Layout.fillWidth: true
                    Layout.maximumWidth: root.window.maximumWidth
                    level: 3
                    font.weight: Font.Bold
                    text: root.mainText
                    wrapMode: Text.Wrap
                    elide: Text.ElideRight
                    horizontalAlignment: Text.AlignHCenter
                }

                QQC2.Label {
                    Layout.topMargin: LingmoUI.Units.largeSpacing
                    Layout.bottomMargin: LingmoUI.Units.largeSpacing
                    Layout.maximumWidth: root.window.maximumWidth
                    Layout.fillWidth: true
                    text: root.subtitle
                    visible: text.length > 0
                    wrapMode: Text.Wrap
                    horizontalAlignment: Text.AlignHCenter
                }

                LingmoUI.Icon {
                    Layout.topMargin: LingmoUI.Units.largeSpacing
                    Layout.alignment: Qt.AlignCenter
                    source: root.iconName
                    implicitWidth: LingmoUI.Units.iconSizes.large
                    implicitHeight: LingmoUI.Units.iconSizes.large
                }

                // content
                QQC2.Control {
                    id: content

                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    Layout.topMargin: LingmoUI.Units.gridUnit
                    Layout.maximumWidth: root.window.maximumWidth

                    leftPadding: 0
                    rightPadding: 0
                    topPadding: 0
                    bottomPadding: 0

                    contentItem: root.mainItem
                    background: Item {}
                }

                QQC2.DialogButtonBox {
                    id: footerButtonBox
                    // ensure we never have no buttons, we always must have the cancel button available
                    standardButtons: (root.standardButtons === QQC2.DialogButtonBox.NoButton) ? QQC2.DialogButtonBox.Cancel : root.standardButtons

                    Layout.topMargin: LingmoUI.Units.largeSpacing
                    Layout.fillWidth: true
                    Layout.maximumWidth: root.window.maximumWidth
                    leftPadding: 0
                    rightPadding: 0
                    topPadding: 0
                    bottomPadding: 0

                    onAccepted: root.window.accept()
                    onRejected: root.window.reject()

                    Repeater {
                        model: root.actions
                        delegate: QQC2.Button {
                            action: modelData
                        }
                    }
                }
            }
        }
    }
}
