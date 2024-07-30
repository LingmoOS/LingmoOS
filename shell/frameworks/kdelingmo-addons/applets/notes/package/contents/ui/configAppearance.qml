/*
 * SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kcmutils as KCM
import org.kde.lingmo.core as LingmoCore
import org.kde.lingmoui 2.20 as LingmoUI
import org.kde.ksvg 1.0 as KSvg
import org.kde.lingmo.components 3.0 as LingmoComponents3
import org.kde.lingmoui 2.5 as LingmoUI
import org.kde.kcmutils as KCM

KCM.GridViewKCM {
    property string cfg_color
    property alias cfg_fontSize: fontSizeSpinBox.value

    header: LingmoUI.FormLayout {
        QQC2.SpinBox {
            id: fontSizeSpinBox

            implicitWidth: LingmoUI.Units.gridUnit * 3
            from: 4
            to: 128
            textFromValue: function (value) {
                return i18n("%1pt", value)
            }
            valueFromText: function (text) {
                return parseInt(text)
            }

            LingmoUI.FormData.label: i18n("Text font size:")
        }
    }

    view.model: ["white", "black", "red", "orange", "yellow", "green", "blue", "pink", "translucent", "translucent-light"]
    view.currentIndex: view.model.indexOf(cfg_color)
    view.onCurrentIndexChanged: cfg_color = view.model[view.currentIndex]

    view.delegate: KCM.GridDelegate {
        id: delegate
        thumbnailAvailable: true
        thumbnail: KSvg.SvgItem {
            anchors.fill: parent
            anchors.margins: LingmoUI.Units.largeSpacing

            imagePath: "widgets/notes"
            elementId: modelData + "-notes"

            QQC2.Label {
                anchors.fill: parent
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter

                text: {
                    switch (modelData) {
                    case "white": return i18n("A white sticky note")
                    case "black": return i18n("A black sticky note")
                    case "red": return i18n("A red sticky note")
                    case "orange": return i18n("An orange sticky note")
                    case "yellow": return i18n("A yellow sticky note")
                    case "green": return i18n("A green sticky note")
                    case "blue": return i18n("A blue sticky note")
                    case "pink": return i18n("A pink sticky note")
                    case "translucent": return i18n("A transparent sticky note")
                    case "translucent-light": return i18n("A transparent sticky note with light text")
                    }
                }
                textFormat: Text.PlainText
                elide: Text.ElideRight
                wrapMode: Text.WordWrap

                //this is deliberately _NOT_ the theme color as we are over a known bright background
                //an unknown colour over a known colour is a bad move as you end up with white on yellow
                color: {
                    if (modelData === "black" || modelData === "translucent-light") {
                        return "#dfdfdf"
                    } else {
                        return "#202020"
                    }
                }
            }
        }
        onClicked: {
            cfg_color = modelData
        }
    }
}
