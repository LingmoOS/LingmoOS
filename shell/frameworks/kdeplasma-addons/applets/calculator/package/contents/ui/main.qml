/*
 *   SPDX-FileCopyrightText: 2015 Bernhard Friedreich <friesoft@gmail.com>
 *   SPDX-FileCopyrightText: 2014 Martin Yrjölä <martin.yrjola@gmail.com>
 *   SPDX-FileCopyrightText: 2012, 2014 Davide Bettio <davide.bettio@kdemail.net>
 *   SPDX-FileCopyrightText: 2012, 2014 David Edmundson <davidedmundson@kde.org>
 *   SPDX-FileCopyrightText: 2012 Luiz Romário Santana Rios <luizromario@gmail.com>
 *   SPDX-FileCopyrightText: 2007 Henry Stanaland <stanaland@gmail.com>
 *   SPDX-FileCopyrightText: 2008 Laurent Montel <montel@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.5
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.5 as QQC2
import org.kde.kirigami 2.20 as Kirigami
import org.kde.ksvg 1.0 as KSvg
import org.kde.plasma.plasma5support 2.0 as P5Support
import org.kde.plasma.components 3.0 as PlasmaComponents
import org.kde.plasma.plasmoid

PlasmoidItem {
    id: main;

    switchWidth: Kirigami.Units.gridUnit * 7
    switchHeight: Math.round(Kirigami.Units.gridUnit * 10.5)

    // Make the buttons' text labels scale with the widget's size
    // This is propagated down to all child controls with text

    property real result: 0;
    property bool hasResult: false;
    property bool showingInput: true;
    property bool showingResult: false;
    property string operator
    property real operand: 0;
    property bool commaPressed: false;
    property int decimals: 0;
    property int inputSize: 0;
    property TextEdit display

    readonly property int maxInputLength: 18; // More than that and the number notation
                                              // turns scientific (i.e.: 1.32324e+12).
                                              // When calculating 1/3 the answer is
                                              // 18 characters long.

    function digitClicked(digit) {
        if (showingResult) {
            allClearClicked();
        }

        if (commaPressed) {
            ++decimals;
            var tenToTheDecimals = Math.pow(10, decimals);
            operand = (operand * tenToTheDecimals + digit) / tenToTheDecimals;
        } else {
            operand = operand * 10 + digit;
        }
        showingInput = true;
        displayNumber(operand);
        ++inputSize;
    }

    function deleteDigit() {
        if (showingResult) {
            allClearClicked();
        }

        if (showingInput) {
            if (commaPressed) {
                if (decimals === 0) {
                    commaPressed = false;
                } else if (decimals > 0) {
                    operand -= operand % Math.pow(10, 1 - decimals);
                    --decimals;
                    --inputSize;
                }
            } else if (inputSize > 0) {
                operand = (operand - (operand % 10)) / 10;
                --inputSize;
            }
        }
        displayNumber(operand);
    }

    function decimalClicked() {
        if (showingResult) {
            allClearClicked();
        }

        commaPressed = true;
        showingInput = true;
        displayNumber(operand);
    }

    function doOperation() {
        switch (operator) {
        case "+":
            result += operand;
            break;
        case "-":
            result -= operand;
            break;
        case "*":
            result *= operand;
            break;
        case "/":
            if (operand === 0) {
                divisionByZero();
                return;
            }
            result /= operand;
            break;
        default:
            return;
        }

        showingInput = false;
        displayNumber(result);
    }

    function clearOperand() {
        operand = 0;
        commaPressed = false;
        decimals = 0;
        inputSize = 0;
    }

    function setOperator(op) {
        if (!hasResult) {
            result = operand;
            hasResult = true;
        } else if (showingInput) {
            doOperation();
        }

        clearOperand();
        operator = op;
        showingResult = false;
    }

    function equalsClicked() {
        showingResult = true;
        doOperation();
    }

    function clearClicked() {
        clearOperand();
        operator = "";
        displayNumber(operand);
        showingInput = true;
        showingResult = false;
    }

    function allClearClicked() {
        clearClicked();
        result = 0;
        hasResult = false;
    }

    function algarismCount(number) {
        return number == 0? 1 :
                            Math.floor(Math.log(Math.abs(number))/Math.log(10)) + 1;
    }

    // use the clipboard datasource for being able to inspect the clipboard content before pasting it
    P5Support.DataSource {
        id: clipboardSource
        property bool editing: false;
        engine: "org.kde.plasma.clipboard"
        connectedSources: "clipboard"
    }

    function copyToClipboard() {
        display.selectAll();
        display.copy();
        display.deselect();
    }

    function pasteFromClipboard() {
        var content = clipboardSource.data["clipboard"]["current"];
        if (content != "") {
            content = content.trim();
        }

        // check if the clipboard content as a whole is a valid number (without sign, no operators, ...)
        if (isValidClipboardInput(content)) {
            var digitRegex = new RegExp('^[0-9]$');
            var decimalRegex = new RegExp('^[\.,]$');

            for (var i = 0; i < content.length; i++) {
                if (digitRegex.test(content[i])) {
                    digitClicked(parseInt(content[i]));
                } else if (decimalRegex.test(content[i])) {
                    decimalClicked();
                }
            }
        }
    }

    function isValidClipboardInput(input) {
        return new RegExp('^[0-9]*[\.,]?[0-9]+$').test(input);
    }

    function displayNumber(number) {
        var text = number.toLocaleString(Qt.locale(), "g", 14);

        // Show all decimals including zeroes and show decimalPoint
        if (showingInput && commaPressed) {
            text = number.toLocaleString(Qt.locale(), "f", decimals);
            if (decimals === 0) {
                text += Qt.locale().decimalPoint;
            }
        }
        display.text = text;

        var decimalsToShow = 9;
        // Decrease precision until the text fits to the display.
        while (display.contentWidth > display.width && decimalsToShow > 0) {
            display.text = number.toLocaleString(Qt.locale(), "g", decimalsToShow--);
        }
    }

    function divisionByZero() {
        showingInput = false;
        showingResult = true;
        display.text = i18nc("Abbreviation for result (undefined) of division by zero, max. six to nine characters.", "undef");
    }

    fullRepresentation: QQC2.Control {
        // Make the buttons' text labels scale with the widget's size
        // This is propagated down to all child controls with text
        font.pixelSize: Math.round(width/12)
        padding: 0
        Layout.minimumWidth: main.switchWidth
        Layout.minimumHeight: main.switchHeight
        Layout.preferredWidth: main.switchWidth * 2
        Layout.preferredHeight: main.switchHeight * 2

        contentItem: ColumnLayout {
            id: mainLayout
            anchors.fill: parent
            anchors.margins: 4

            focus: true;
            spacing: 4;

            Keys.onDigit0Pressed: { digitClicked(0); zeroButton.forceActiveFocus(Qt.TabFocusReason); }
            Keys.onDigit1Pressed: { digitClicked(1); oneButton.forceActiveFocus(Qt.TabFocusReason); }
            Keys.onDigit2Pressed: { digitClicked(2); twoButton.forceActiveFocus(Qt.TabFocusReason); }
            Keys.onDigit3Pressed: { digitClicked(3); threeButton.forceActiveFocus(Qt.TabFocusReason); }
            Keys.onDigit4Pressed: { digitClicked(4); fourButton.forceActiveFocus(Qt.TabFocusReason); }
            Keys.onDigit5Pressed: { digitClicked(5); fiveButton.forceActiveFocus(Qt.TabFocusReason); }
            Keys.onDigit6Pressed: { digitClicked(6); sixButton.forceActiveFocus(Qt.TabFocusReason); }
            Keys.onDigit7Pressed: { digitClicked(7); sevenButton.forceActiveFocus(Qt.TabFocusReason); }
            Keys.onDigit8Pressed: { digitClicked(8); eightButton.forceActiveFocus(Qt.TabFocusReason); }
            Keys.onDigit9Pressed: { digitClicked(9); nineButton.forceActiveFocus(Qt.TabFocusReason); }
            Keys.onEscapePressed: { allClearClicked(); allClearButton.forceActiveFocus(Qt.TabFocusReason); }
            Keys.onDeletePressed: { clearClicked(); clearButton.forceActiveFocus(Qt.TabFocusReason); }
            Keys.onPressed: {
                switch (event.key) {
                case Qt.Key_Plus:
                    setOperator("+");
                    plusButton.forceActiveFocus(Qt.TabFocusReason);
                    break;
                case Qt.Key_Minus:
                    setOperator("-");
                    minusButton.forceActiveFocus(Qt.TabFocusReason);
                    break;
                case Qt.Key_Asterisk:
                    setOperator("*");
                    multiplyButton.forceActiveFocus(Qt.TabFocusReason);
                    break;
                case Qt.Key_Slash:
                    setOperator("/");
                    divideButton.forceActiveFocus(Qt.TabFocusReason);
                    break;
                case Qt.Key_Comma:
                case Qt.Key_Period:
                    decimalClicked();
                    decimalButton.forceActiveFocus(Qt.TabFocusReason);
                    break;
                case Qt.Key_Equal:
                case Qt.Key_Return:
                case Qt.Key_Enter:
                    equalsClicked();
                    break;
                case Qt.Key_Backspace:
                    deleteDigit();
                    display.forceActiveFocus(Qt.TabFocusReason);
                    break;
                default:
                    if (event.matches(StandardKey.Copy)) {
                        copyToClipboard();
                    } else if (event.matches(StandardKey.Paste)) {
                        pasteFromClipboard();
                    }
                    break;
                }
            }

            KeyNavigation.up: zeroButton
            KeyNavigation.down: clearButton
            KeyNavigation.left: allClearButton
            KeyNavigation.right: clearButton

            KSvg.FrameSvgItem {
                id: displayFrame;
                Layout.fillWidth: true
                Layout.minimumHeight: 2 * display.font.pixelSize;
                imagePath: "widgets/frame";
                prefix: "plain";

                TextEdit {
                    id: display;
                    anchors {
                        fill: parent;
                        margins: parent.margins.right;
                    }
                    text: "0";
                    font.pointSize: Kirigami.Theme.defaultFont.pointSize * 2;
                    font.weight: Font.Bold;
                    Kirigami.Theme.colorSet: Kirigami.Theme.View
                    color: Kirigami.Theme.textColor
                    horizontalAlignment: TextEdit.AlignRight;
                    verticalAlignment: TextEdit.AlignVCenter;
                    readOnly: true;

                    focus: main.expanded

                    Accessible.name: text
                    Accessible.description: i18nc("@label calculation result", "Result")

                    Binding {
                        target: main
                        property: "display"
                        value: display
                    }
                }
            }

            GridLayout {
                id: buttonsGrid;
                columns: 4;
                rows: 5;
                columnSpacing: 4
                rowSpacing: 4

                Layout.fillWidth: true
                Layout.fillHeight: true

                PlasmaComponents.Button {
                    id: clearButton

                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    KeyNavigation.down: sevenButton
                    KeyNavigation.right: divideButton

                    text: i18nc("Text of the clear button", "C");
                    onClicked: clearClicked();
                }

                PlasmaComponents.Button {
                    id: divideButton

                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    KeyNavigation.down: eightButton
                    KeyNavigation.right: multiplyButton

                    text: i18nc("Text of the division button", "÷");
                    onClicked: setOperator("/");
                }

                PlasmaComponents.Button {
                    id: multiplyButton

                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    KeyNavigation.down: nineButton
                    KeyNavigation.right: allClearButton

                    text: i18nc("Text of the multiplication button", "×");
                    onClicked: setOperator("*");
                }

                PlasmaComponents.Button {
                    id: allClearButton

                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    KeyNavigation.down: minusButton

                    text: i18nc("Text of the all clear button", "AC");
                    onClicked: allClearClicked();
                }


                PlasmaComponents.Button {
                    id: sevenButton

                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    KeyNavigation.down: fourButton
                    KeyNavigation.right: eightButton

                    text: "7";
                    onClicked: digitClicked(7);
                }

                PlasmaComponents.Button {
                    id: eightButton

                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    KeyNavigation.down: fiveButton
                    KeyNavigation.right: nineButton

                    text: "8";
                    onClicked: digitClicked(8);
                }

                PlasmaComponents.Button {
                    id: nineButton

                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    KeyNavigation.down: sixButton
                    KeyNavigation.right: minusButton

                    text: "9";
                    onClicked: digitClicked(9);
                }

                PlasmaComponents.Button {
                    id: minusButton

                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    KeyNavigation.down: plusButton

                    text: i18nc("Text of the minus button", "-");
                    onClicked: setOperator("-");
                }


                PlasmaComponents.Button {
                    id: fourButton

                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    KeyNavigation.down: oneButton
                    KeyNavigation.right: fiveButton

                    text: "4";
                    onClicked: digitClicked(4);
                }

                PlasmaComponents.Button {
                    id: fiveButton

                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    KeyNavigation.down: twoButton
                    KeyNavigation.right: sixButton

                    text: "5";
                    onClicked: digitClicked(5);
                }

                PlasmaComponents.Button {
                    id: sixButton

                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    KeyNavigation.down: threeButton
                    KeyNavigation.right: plusButton

                    text: "6";
                    onClicked: digitClicked(6);
                }

                PlasmaComponents.Button {
                    id: plusButton

                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    KeyNavigation.down: ansButton

                    text: i18nc("Text of the plus button", "+");
                    onClicked: setOperator("+");
                }


                PlasmaComponents.Button {
                    id: oneButton

                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    KeyNavigation.down: zeroButton
                    KeyNavigation.right: twoButton

                    text: "1";
                    onClicked: digitClicked(1);
                }

                PlasmaComponents.Button {
                    id: twoButton

                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    KeyNavigation.down: zeroButton
                    KeyNavigation.right: threeButton

                    text: "2";
                    onClicked: digitClicked(2);
                }

                PlasmaComponents.Button {
                    id: threeButton

                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    KeyNavigation.down: decimalButton
                    KeyNavigation.right: ansButton

                    text: "3";
                    onClicked: digitClicked(3);
                }

                PlasmaComponents.Button {
                    id: ansButton

                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    Layout.rowSpan: 2
                    text: i18nc("Text of the equals button", "=");
                    onClicked: equalsClicked();
                }

                PlasmaComponents.Button {
                    id: zeroButton

                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    KeyNavigation.right: decimalButton

                    Layout.columnSpan: 2
                    text: "0";
                    onClicked: digitClicked(0);
                }

                PlasmaComponents.Button {
                    id: decimalButton

                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    KeyNavigation.right: ansButton

                    text: Qt.locale().decimalPoint;
                    onClicked: decimalClicked();
                }
            }
        }
    }
}

