/*
 *  SPDX-FileCopyrightText: 2024 ivan tkachenko <me@ratijas.tk>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.lingmoui as LingmoUI

ColumnLayout {
    id: root

    required property list<LingmoUI.imageColorsPaletteSwatch> swatches

    spacing: LingmoUI.Units.smallSpacing

    RowLayout {
        spacing: LingmoUI.Units.smallSpacing

        QQC2.Label {
            text: "Color"
        }

        Item {
            Layout.fillWidth: true
        }

        QQC2.Label {
            text: "Contrast Color"
            elide: Text.ElideRight
        }
    }

    Repeater {
        model: root.swatches

        delegate: RowLayout {
            id: delegate

            required property LingmoUI.imageColorsPaletteSwatch modelData

            spacing: LingmoUI.Units.smallSpacing
            Layout.fillWidth: true

            Item {
                implicitHeight: colorItem.height
                Layout.fillWidth: true

                ColorWell {
                    id: colorItem
                    anchors.left: parent.left
                    width: Math.max(12, parent.width * delegate.modelData.ratio)
                    color: delegate.modelData.color
                    showLabel: false
                }

                ShadowedLabel {
                    id: label

                    anchors {
                        top: parent.top
                        left: colorItem.right
                        bottom: parent.bottom
                    }
                    verticalAlignment: Text.AlignVCenter
                    text: `${(delegate.modelData.ratio * 100).toFixed(2)}%`

                    states: [
                        State {
                            when: delegate.modelData.ratio > 0.66
                            name: "inside"

                            PropertyChanges {
                                target: label
                                anchors.leftMargin: - label.width - LingmoUI.Units.largeSpacing
                                layer.enabled: true
                                // The contrast color sometimes isn't really usable
                                // color: delegate.modelData.contrastColor
                            }
                        },
                        State {
                            when: true
                            name: "side-by-side"

                            PropertyChanges {
                                target: label
                                anchors.leftMargin: LingmoUI.Units.smallSpacing
                            }
                        }
                    ]
                }
            }
            ColorWell {
                Layout.fillWidth: false
                Layout.alignment: Qt.AlignRight
                color: delegate.modelData.contrastColor
                showLabel: false
            }
        }
    }
}
