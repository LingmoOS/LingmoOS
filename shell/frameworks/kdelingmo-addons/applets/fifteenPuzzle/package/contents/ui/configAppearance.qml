/*
 * SPDX-FileCopyrightText: 2013 Bhushan Shah <bhush94@gmail.com>
 * SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>
 * SPDX-FileCopyrightText: 2014 Kai Uwe Broulik <kde@privat.broulik.de>
 * SPDX-FileCopyrightText: 2014 Jeremy Whiting <jpwhiting@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick 2.15
import QtCore
import QtQuick.Controls 2.15 as QtControls
import QtQuick.Layouts 1.15
import QtQuick.Dialogs as QtDialogs

import org.kde.kquickcontrols 2.0 as KQC
import org.kde.lingmoui 2.20 as LingmoUI
import org.kde.kcmutils as KCM

KCM.SimpleKCM {
    property alias cfg_boardSize: sizeSpinBox.value
    property alias cfg_boardColor: pieceColorPicker.color
    property alias cfg_numberColor: numberColorPicker.color
    property alias cfg_showNumerals: showNumeralsCheckBox.checked

    property alias cfg_useImage: imageBackgroundRadioButton.checked
    property alias cfg_imagePath: imagePathTextField.text

    LingmoUI.FormLayout {

        QtControls.ButtonGroup {
            id: radioGroup
        }

        // Need to manually set checked state for the color button based on the
        // other onebecause it's not aliased to a saved property
        Component.onCompleted: {
            colorBackgroundRadioButton.checked = !imageBackgroundRadioButton.checked;
        }

        QtControls.SpinBox {
            id: sizeSpinBox
            LingmoUI.FormData.label: i18nc("@label:spinbox", "Grid size:")
        }

        Item {
            LingmoUI.FormData.isSection: true
        }

        // Color background
        RowLayout {
            LingmoUI.FormData.label: i18n("Background:")

            QtControls.RadioButton {
                id: colorBackgroundRadioButton
                QtControls.ButtonGroup.group: radioGroup

                text: i18n("Color:")
            }

            KQC.ColorButton {
                id: pieceColorPicker
                enabled: colorBackgroundRadioButton.checked
            }
        }

        // Image background
        RowLayout {
            QtControls.RadioButton {
                id: imageBackgroundRadioButton
                QtControls.ButtonGroup.group: radioGroup

                text: i18n("Image:")
            }

            LingmoUI.ActionTextField {
                id: imagePathTextField
                enabled: imageBackgroundRadioButton.checked

                Layout.fillWidth: true
                placeholderText: i18nc("@info:placeholder", "Path to custom image…")

                rightActions: [
                    LingmoUI.Action {
                        icon.name: "edit-clear"
                        visible: imagePathTextField.text.length !== 0
                        onTriggered: imagePathTextField.text = "";
                    }
                ]
            }

            QtControls.Button {
                id: imageButton
                enabled: imageBackgroundRadioButton.checked

                icon.name: "document-open"

                QtControls.ToolTip {
                    visible: imageButton.hovered
                    text: i18nc("@info:tooltip", "Choose image…")
                }

                onClicked: imagePicker.open()

                QtDialogs.FileDialog {
                    id: imagePicker

                    title: i18nc("@title:window", "Choose an Image")

                    currentFolder: StandardPaths.standardLocations(StandardPaths.PicturesLocation)[0]

                    // TODO ask QImageReader for supported formats
                    nameFilters: [ i18n("Image Files (*.png *.jpg *.jpeg *.bmp *.svg *.svgz)") ]

                    onAccepted: {
                        imagePathTextField.text = selectedFile.toString().replace("file://", "")
                    }
                }
            }
        }

        Item {
            LingmoUI.FormData.isSection: true
        }

        RowLayout {
            LingmoUI.FormData.label: i18n("Tiles:")

            QtControls.CheckBox {
                id: showNumeralsCheckBox
                text: i18n("Colored numbers:")
            }

            KQC.ColorButton {
                id: numberColorPicker
                enabled: showNumeralsCheckBox.checked
            }
        }
    }
}
