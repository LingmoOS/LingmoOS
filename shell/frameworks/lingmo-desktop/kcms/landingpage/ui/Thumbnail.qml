/*
    SPDX-FileCopyrightText: 2021 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2018 Furkan Tokac <furkantokac34@gmail.com>
    SPDX-FileCopyrightText: 2019 Nate Graham <nate@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
import QtQuick 2.7
import QtQuick.Controls 2.5 as QQC2
import QtQuick.Layouts 1.3
import QtQuick.Window 2.2
import org.kde.lingmoui as LingmoUI

QQC2.RadioButton {
    id: delegate

    property alias imageSource: image.source

    implicitWidth: contentItem.implicitWidth
    implicitHeight: contentItem.implicitHeight

    contentItem: ColumnLayout {
        spacing: 0

        LingmoUI.ShadowedRectangle {
            implicitWidth: implicitHeight * 1.6
            implicitHeight: LingmoUI.Units.gridUnit * 5
            radius: LingmoUI.Units.cornerRadius
            LingmoUI.Theme.inherit: false
            LingmoUI.Theme.colorSet: LingmoUI.Theme.View

            shadow.xOffset: 0
            shadow.yOffset: 2
            shadow.size: 10
            shadow.color: Qt.rgba(0, 0, 0, 0.3)

            color: {
                if (delegate.checked) {
                    return LingmoUI.Theme.highlightColor;
                } else if (delegate.hovered) {
                    // Match appearance of hovered list items
                    return Qt.rgba(LingmoUI.Theme.highlightColor.r,
                                LingmoUI.Theme.highlightColor.g,
                                LingmoUI.Theme.highlightColor.b,
                                0.5);
                } else {
                    return LingmoUI.Theme.backgroundColor;
                }
            }

            Image {
                id: image
                anchors {
                    fill: parent
                    margins: LingmoUI.Units.smallSpacing
                }
                asynchronous: true
                sourceSize: Qt.size(width * Screen.devicePixelRatio,
                                    height * Screen.devicePixelRatio)
            }
        }

        QQC2.Label {
            id: label
            Layout.fillWidth: true
            Layout.topMargin: LingmoUI.Units.smallSpacing
            text: delegate.text
            textFormat: Text.PlainText
            horizontalAlignment: Text.AlignHCenter
        }

        Rectangle {
            Layout.preferredWidth: label.paintedWidth
            Layout.preferredHeight: 1
            Layout.alignment: Qt.AlignHCenter

            color: LingmoUI.Theme.highlightColor

            opacity: delegate.visualFocus ? 1 : 0
        }
    }

    indicator: Item {}
    background: Item {}
}
