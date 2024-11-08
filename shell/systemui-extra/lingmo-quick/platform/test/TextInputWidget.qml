import QtQuick 2.15
import org.lingmo.quick.platform 1.0 as Platform
import org.lingmo.quick.items 1.0

Item {
    property alias text: textInput.text;
    StyleBackground {
        anchors.fill: parent
        alpha: 0.04
        useStyleTransparency: false
        paletteRole: Platform.Theme.Text
        border.width: 1
        borderAlpha: textInput.activeFocus ? 1 : 0.1
        borderColor: textInput.activeFocus ? Platform.Theme.Highlight : Platform.Theme.BrightText
        z: 0
    }
    TextInput {
        id: textInput
        clip: true
        anchors.fill: parent
        selectByMouse: true
        verticalAlignment: TextInput.AlignVCenter
        font.pointSize: Platform.Theme.fontSize
        activeFocusOnPress: true;
        leftPadding: 2
        rightPadding: 2
        activeFocusOnTab: true;
        z: 1

        function updateTextInputColor() {
            color = Platform.Theme.text();
            selectionColor = Platform.Theme.highlight();
        }

        Platform.Theme.onPaletteChanged: {
            updateTextInputColor();
        }

        Component.onCompleted: {
            updateTextInputColor();
        }
    }
}