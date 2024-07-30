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
import "fakecontrols" as FakeControls


Dialog {
    id: dialog
    property alias textColor: textButton.color
    property alias backgroundColor: backgroundButton.color
    property alias highlightColor: highlightButton.color
    property alias linkColor: linkButton.color
    property alias visitedLinkColor: visitedLinkButton.color

    property alias buttonTextColor: buttonTextButton.color
    property alias buttonBackgroundColor: buttonBackgroundButton.color
    property alias buttonHoverColor: buttonHoverButton.color
    property alias buttonFocusColor: buttonFocusButton.color

    property alias viewTextColor: viewTextButton.color
    property alias viewBackgroundColor: viewBackgroundButton.color
    property alias viewHoverColor: viewHoverButton.color
    property alias viewFocusColor: viewFocusButton.color

    property alias complementaryTextColor: complementaryTextButton.color
    property alias complementaryBackgroundColor: complementaryBackgroundButton.color
    property alias complementaryHoverColor: complementaryHoverButton.color
    property alias complementaryFocusColor: complementaryFocusButton.color

    title: i18n("Edit Colors");

    onVisibleChanged: {
        if (visible) {
            textColor = themeModel.colorEditor.textColor;
            backgroundColor = themeModel.colorEditor.backgroundColor;
            highlightColor = themeModel.colorEditor.highlightColor;
            linkColor = themeModel.colorEditor.linkColor;
            visitedLinkColor = themeModel.colorEditor.visitedLinkColor;

            buttonTextColor = themeModel.colorEditor.buttonTextColor;
            buttonBackgroundColor = themeModel.colorEditor.buttonBackgroundColor;
            buttonHoverColor = themeModel.colorEditor.buttonHoverColor;
            buttonFocusColor = themeModel.colorEditor.buttonFocusColor;

            viewTextColor = themeModel.colorEditor.viewTextColor;
            viewBackgroundColor = themeModel.colorEditor.viewBackgroundColor;
            viewHoverColor = themeModel.colorEditor.viewHoverColor;
            viewFocusColor = themeModel.colorEditor.viewFocusColor;

            complementaryTextColor = themeModel.colorEditor.complementaryTextColor;
            complementaryBackgroundColor = themeModel.colorEditor.complementaryBackgroundColor;
            complementaryHoverColor = themeModel.colorEditor.complementaryHoverColor;
            complementaryFocusColor = themeModel.colorEditor.complementaryFocusColor;
        }
    }
    ColorDialog {
        id: colorDialog
        modality: Qt.WindowModal
        title: i18n("Select Color")
        property Item activeButton
        onAccepted: {
            activeButton.color = color;
        }
    }
    contentItem: Rectangle {
        implicitWidth:  Kirigami.Units.gridUnit * 50
        implicitHeight: Kirigami.Units.gridUnit * 42

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
            anchors.fill: parent
            spacing: 0
            Rectangle {
                Layout.fillWidth: true
                //Layout.fillHeight: true
                Layout.minimumHeight: Kirigami.Units.gridUnit * 15
                color: palette.base
                Kirigami.ShadowedRectangle {
                    id: plasmoidPreview
                    anchors.centerIn: parent
                    width: parent.width/1.5
                    height: parent.height/1.5
                    radius: Kirigami.Units.smallSpacing
                    color: backgroundColor
                    shadow {
                        xOffset: 0
                        yOffset: Kirigami.Units.smallSpacing/2
                        size: Kirigami.Units.gridUnit / 2.2
                        color: Qt.rgba(0, 0, 0, 0.5)
                    }
                    ColumnLayout {
                        anchors.centerIn: parent
                        Label {
                            Layout.alignment: Qt.AlignHCenter
                            text: i18n("Normal text")
                            color: textColor
                        }
                        RowLayout {
                            Layout.alignment: Qt.AlignHCenter
                            Label {
                                text: i18n("Link")
                                color: linkColor
                            }
                            Label {
                                text: i18n("Visited Link")
                                color: visitedLinkColor
                            }
                        }
                        FakeControls.CheckBox {
                            Layout.alignment: Qt.AlignHCenter
                        }
                        FakeControls.LineEdit {
                            Layout.alignment: Qt.AlignHCenter
                        }
                        FakeControls.Button {
                            Layout.alignment: Qt.AlignHCenter
                        }
                    }
                }
            }
            Rectangle {
                id: complementaryBar
                color: complementaryBackgroundColor
                height: Kirigami.Units.gridUnit * 2
                Layout.fillWidth: true

                Label {
                    text: i18n("Complementary colors area:")
                    color: complementaryTextColor
                    anchors {
                        verticalCenter: parent.verticalCenter
                        left: complementaryBar.left
                        leftMargin: Kirigami.Units.smallspacing
                    }
                }
                RowLayout {
                    anchors {
                        top: complementaryBar.top
                        bottom: complementaryBar.bottom
                        horizontalCenter: complementaryBar.horizontalCenter
                    }
                    Label {
                        text: i18n("Label")
                        color: complementaryTextColor
                    }
                    FakeControls.CheckBox {
                        complementary: true
                    }
                }
            }
            ScrollView {
                id: scroll
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.minimumHeight: Kirigami.Units.gridUnit * 15
                Item {
                    width: parent.width
                    implicitHeight: childrenRect.height
                    GridLayout {
                        //Layout.alignment: Qt.AlignHCenter
                        anchors.centerIn: parent
                        columns: 2
                        columnSpacing: Kirigami.Units.smallSpacing

                        FormLabel {
                            text: i18n("Text color:")
                            buddy: textButton
                        }
                        ColorButton {
                            id: textButton
                        }
                        FormLabel {
                            text: i18n("Background color:")
                            buddy: backgroundButton
                        }
                        ColorButton {
                            id: backgroundButton
                        }
                        FormLabel {
                            text: i18n("Highlight color:")
                            buddy: highlightButton
                        }
                        ColorButton {
                            id: highlightButton
                        }
                        FormLabel {
                            text: i18n("Link color:")
                            buddy: linkButton
                        }
                        ColorButton {
                            id: linkButton
                        }
                        FormLabel {
                            text: i18n("Visited link color:")
                            buddy: visitedLinkButton
                        }
                        ColorButton {
                            id: visitedLinkButton
                        }

                        FormLabel {
                            text: i18n("Button text color:")
                            buddy: buttonTextButton
                        }
                        ColorButton {
                            id: buttonTextButton
                        }
                        FormLabel {
                            text: i18n("Button background color:")
                            buddy: buttonBackgroundButton
                        }
                        ColorButton {
                            id: buttonBackgroundButton
                        }
                        FormLabel {
                            text: i18n("Button mouse over color:")
                            buddy: buttonHoverButton
                        }
                        ColorButton {
                            id: buttonHoverButton
                        }
                        FormLabel {
                            text: i18n("Button focus color:")
                            buddy: buttonFocusButton
                        }
                        ColorButton {
                            id: buttonFocusButton
                        }

                        FormLabel {
                            text: i18n("Text view text color:")
                            buddy: viewTextButton
                        }
                        ColorButton {
                            id: viewTextButton
                        }
                        FormLabel {
                            text: i18n("Text view background color:")
                            buddy: viewBackgroundButton
                        }
                        ColorButton {
                            id: viewBackgroundButton
                        }
                        FormLabel {
                            text: i18n("Text view mouse over color:")
                            buddy: viewHoverButton
                        }
                        ColorButton {
                            id: viewHoverButton
                        }
                        FormLabel {
                            text: i18n("Text view focus color:")
                            buddy: viewFocusButton
                        }
                        ColorButton {
                            id: viewFocusButton
                        }

                        FormLabel {
                            text: i18n("Complementary text color:")
                            buddy: complementaryTextButton
                        }
                        ColorButton {
                            id: complementaryTextButton
                        }
                        FormLabel {
                            text: i18n("Complementary background color:")
                            buddy: complementaryBackgroundButton
                        }
                        ColorButton {
                            id: complementaryBackgroundButton
                        }
                        FormLabel {
                            text: i18n("Complementary mouse over color:")
                            buddy: complementaryHoverButton
                        }
                        ColorButton {
                            id: complementaryHoverButton
                        }
                        FormLabel {
                            text: i18n("Complementary focus color:")
                            buddy: complementaryFocusButton
                        }
                        ColorButton {
                            id: complementaryFocusButton
                        }
                    }
                }
            }
            DialogButtonBox {
                Layout.fillWidth: true
                Button {
                    text: i18n("OK")
                    onClicked: dialog.accept()
                    DialogButtonBox.buttonRole: DialogButtonBox.AcceptRole
                }
                Button {
                    text: i18n("Cancel")
                    onClicked: dialog.reject()
                    DialogButtonBox.buttonRole: DialogButtonBox.DestructiveRole
                }
            }
        }
    }

    onAccepted: {
        themeModel.colorEditor.textColor = textColor;
        themeModel.colorEditor.backgroundColor = backgroundColor;
        themeModel.colorEditor.highlightColor = highlightColor;
        themeModel.colorEditor.linkColor = linkColor;
        themeModel.colorEditor.visitedLinkColor = visitedLinkColor;

        themeModel.colorEditor.buttonTextColor = buttonTextColor;
        themeModel.colorEditor.buttonBackgroundColor = buttonBackgroundColor;
        themeModel.colorEditor.buttonHoverColor = buttonHoverColor;
        themeModel.colorEditor.buttonFocusColor = buttonFocusColor;

        themeModel.colorEditor.viewTextColor = viewTextColor;
        themeModel.colorEditor.viewBackgroundColor = viewBackgroundColor;
        themeModel.colorEditor.viewHoverColor = viewHoverColor;
        themeModel.colorEditor.viewFocusColor = viewFocusColor;

        themeModel.colorEditor.complementaryTextColor = complementaryTextColor;
        themeModel.colorEditor.complementaryBackgroundColor = complementaryBackgroundColor;
        themeModel.colorEditor.complementaryHoverColor = complementaryHoverColor;
        themeModel.colorEditor.complementaryFocusColor = complementaryFocusColor;

        themeModel.colorEditor.save();
    }
}
