import QtQuick 2.9
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.5


import LingmoUI 1.0 as CuteUI
import Lingmo.Calculator 1.0

Item {
    id: zone

    ColumnLayout {
        id: layout
        anchors.fill: parent
        anchors.margins: 0

        ListView {
            id: listView
            model: ListModel { id: historyModel }
            clip: true

            Layout.fillHeight: true
            Layout.fillWidth: true

            flickableDirection: Flickable.VerticalFlick
            ScrollBar.vertical: ScrollBar {}

            onCountChanged: {
                listView.currentIndex = count - 1
            }

            delegate: Item {
                height: label.implicitHeight + CuteUI.Units.largeSpacing * 2
                width: parent ? parent.width : undefined

                Label {
                    id: label
                    anchors.fill: parent
                    horizontalAlignment: Qt.AlignRight
                    text: historyModel.count > 0 ? historyModel.get(index).text : ""
                    elide: Text.ElideMiddle
                    color: CuteUI.Theme.disabledTextColor

                    leftPadding: CuteUI.Units.largeSpacing
                    rightPadding: CuteUI.Units.largeSpacing

                    MouseArea {
                        hoverEnabled: true
                    }
                }
            }
        }

        TextField {
            id: textField
            Layout.preferredHeight: 50
            Layout.fillWidth: true
            Keys.onReturnPressed: appendToTextField('=')
            Keys.onEnterPressed: appendToTextField('=')

            selectByMouse: true
            horizontalAlignment: TextInput.AlignRight
            focus: Qt.StrongFocus
            font.pixelSize: 24

            background: Rectangle {
                border.width: 0
                color: "transparent"
            }

            leftPadding: CuteUI.Units.largeSpacing
            rightPadding: CuteUI.Units.largeSpacing
        }
    }

    function appendToTextField(text) {
        if (text === '=') {
            var res = calculate(textField.text)
            if (res !== '') {
                var expressionText = textField.text
                textField.text = res

                // If the expression and the result area equal,
                // it will not be added to the ListView
                if (expressionText !== res) {
                    expressionText = expressionText + " = " + res
                    historyModel.append({"text": expressionText})
                }
            }
        } else if (text === 'C') {
            if (textField.text != "")
                textField.clear()
            else 
                historyModel.clear()
        } else if (text === 'BACKSPACE') {
            // backspace
            textField.remove(textField.cursorPosition, textField.cursorPosition - 1)
        } else {
            textField.insert(textField.cursorPosition, text)
        }
    }
}
