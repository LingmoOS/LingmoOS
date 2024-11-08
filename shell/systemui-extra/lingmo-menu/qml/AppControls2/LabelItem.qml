import QtQuick 2.0
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.5
import org.lingmo.menu.core 1.0
import org.lingmo.quick.platform 1.0 as Platform
import org.lingmo.quick.items 1.0 as LingmoUIItems

MouseArea {
    id: control
    hoverEnabled: true

    property alias displayName: labelText.text

    // ToolTip.text: comment
    // ToolTip.visible: control.containsMouse
    // ToolTip.delay: 500

    LingmoUIItems.StyleBackground {
        anchors.fill: parent
        radius: Platform.Theme.minRadius
        useStyleTransparency: false
        paletteRole: Platform.Theme.Text
        alpha: control.containsPress ? 0.15 : control.containsMouse ? 0.08 : 0.00

        RowLayout {
            anchors.fill: parent
            LingmoUIItems.StyleText {
                id: labelText
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.leftMargin: 12

                horizontalAlignment: Qt.AlignLeft
                verticalAlignment: Qt.AlignVCenter
                font.bold: true
                elide: Text.ElideRight
            }

            LingmoUIItems.Icon {
                visible: control.containsMouse
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                Layout.preferredWidth: 24
                Layout.preferredHeight: 24
                Layout.rightMargin: 16
                mode: LingmoUIItems.Icon.AutoHighlight
                source: "open-menu-symbolic"
            }
        }
    }
}
