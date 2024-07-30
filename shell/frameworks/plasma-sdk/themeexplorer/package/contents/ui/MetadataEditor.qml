/*
 *   SPDX-FileCopyrightText: 2015 Marco Martin <mart@kde.org>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick 2.3
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.15
import QtQuick.Dialogs
import org.kde.kirigami as Kirigami

Dialog {
    id: dialog
    property alias name: nameField.text
    property alias author: authorField.text
    property alias email: emailField.text
    property alias license: licenseField.editText
    property alias website: websiteField.text
    property bool newTheme: false

    property bool canEdit: false

    title: newTheme ? i18n("New Theme") : i18n("Edit Theme")

    onVisibleChanged: {
        nameField.focus = true
    }

    //all this reimplementing shouldn't be necessary,
    //but unfortunately native standard buttons management
    //is completely broken
    contentItem: Rectangle {
        implicitWidth:  layout.Layout.minimumWidth + Kirigami.Units.smallSpacing*2
        implicitHeight: layout.Layout.minimumHeight + Kirigami.Units.smallSpacing*2

        Keys.onPressed: {
            if (event.key == Qt.Key_Enter || event.key == Qt.Key_Return) {
                dialog.accept();
            } else if (event.key == Qt.Key_Escape) {
                dialog.reject();
            }
        }

        SystemPalette {
            id: palette
        }
        color: palette.window

        ColumnLayout {
            id: layout
            anchors {
                fill: parent
                margins: Kirigami.Units.smallSpacing
            }
            Label {
                id: errorMessage
                text: ""
                property string defaultMEssage: newTheme ? "" : i18n("Warning: don't change author or license for themes you don't own")
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }
            GridLayout {
                Layout.fillWidth: true
                columns: 2
                columnSpacing: Kirigami.Units.smallSpacing

                FormLabel {
                    visible: newTheme
                    text: i18n("Theme Name:")
                    buddy: nameField
                }
                TextField {
                    id: nameField
                    visible: newTheme
                    Layout.fillWidth: true
                    onTextChanged: {
                        if (!newTheme) {
                            errorMessage.text = errorMessage.defaultMEssage;
                            dialog.canEdit = true;
                            return;
                        }
                        for (var i = 0; i < themeModel.themeList.count; ++i) {
                            if (nameField.text == themeModel.themeList.get(i).packageNameRole) {
                                dialog.canEdit = false;
                                errorMessage.text = i18n("This theme name already exists");
                                return;
                            }
                        }
                        errorMessage.text = "";
                        dialog.canEdit = true;
                    }
                }
                FormLabel {
                    text: i18n("Author:")
                    buddy: authorField
                }
                TextField {
                    id: authorField
                    Layout.fillWidth: true
                }
                FormLabel {
                    text: i18n("Email:")
                    buddy: emailField
                }
                TextField {
                    id: emailField
                    Layout.fillWidth: true
                }
                FormLabel {
                    text: i18n("License:")
                    buddy: licenseField
                }
                ComboBox {
                    id: licenseField
                    Layout.fillWidth: true
                    editable: true
                    editText: "LGPL 2.1+"
                    model: ["LGPL 2.1+", "GPL 2+", "GPL 3+", "LGPL 3+", "BSD"]
                }
                FormLabel {
                    text: i18n("Website:")
                    buddy: websiteField
                }
                TextField {
                    id: websiteField
                    Layout.fillWidth: true
                }
            }
            Item {
                Layout.fillHeight: true
            }
            DialogButtonBox {
                Layout.alignment: Qt.AlignRight
                Button {
                    DialogButtonBox.buttonRole: DialogButtonBox.AcceptRole
                    text: i18n("OK")
                    onClicked: dialog.accept()
                    enabled: canEdit && nameField.text && authorField.text && emailField.text && websiteField.text
                }
                Button {
                    DialogButtonBox.buttonRole: DialogButtonBox.DestructiveRole
                    text: i18n("Cancel")
                    onClicked: dialog.reject()
                }
            }
        }
    }

    onAccepted: {
        if (newTheme) {
            themeModel.createNewTheme(nameField.text, authorField.text, emailField.text, licenseField.editText, websiteField.text);
            for (var i = 0; i < themeModel.themeList.count; ++i) {
                if (nameField.text == themeModel.themeList.get(i).packageNameRole) {
                    themeSelector.currentIndex = i;
                    break;
                }
            }
        } else {
            themeModel.editThemeMetaData(nameField.text, authorField.text, emailField.text, licenseField.editText, websiteField.text);
        }
    }
}
