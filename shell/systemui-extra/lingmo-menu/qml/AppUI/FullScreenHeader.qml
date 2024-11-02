import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12
import org.lingmo.quick.platform 1.0 as Platform
import org.lingmo.quick.items 1.0 as LingmoUIItems
import org.lingmo.menu.core 1.0
import org.lingmo.menu.utils 1.0

Item {
    clip: true
    property alias searchOpacity: searchInputBar.opacity
    property alias searchText: searchInputBar.text

    PluginSelectButton {
        id: pluginSelectButton
        anchors.left: parent.left
        anchors.leftMargin: 72
        anchors.verticalCenter: parent.verticalCenter
        height: 32; width: 64
    }

    SearchInputBar {
        id: searchInputBar
        width: 372; height: 36
        anchors.centerIn: parent
        radius: Platform.Theme.minRadius
        visible: opacity

        onTextChanged: {
            if (text === "") {
                // appPageHeaderUtils.model(PluginGroup.SortMenuItem).reactivateProvider();
            } else {
                // appPageHeaderUtils.model(PluginGroup.Button).reactivateProvider();
                // appPageHeaderUtils.startSearch(text);
            }
        }
        Behavior on opacity {
            NumberAnimation { duration: 300; easing.type: Easing.InOutCubic }
        }
    }

    LingmoUIItems.StyleBackground {
        width: 48; height: width
        radius: Platform.Theme.minRadius
        useStyleTransparency: false
        paletteRole: Platform.Theme.Light
        alpha: buttonMouseArea.containsPress ? 0.30 : buttonMouseArea.containsMouse ? 0.20 : 0.00
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        borderColor: Platform.Theme.Highlight
        border.width: buttonMouseArea.activeFocus ? 2 : 0

        LingmoUIItems.Icon {
            anchors.centerIn: parent
            width: parent.width / 2; height: width
            source: "view-restore-symbolic"
            mode: LingmoUIItems.Icon.Highlight
        }
        MouseArea {
            id: buttonMouseArea
            hoverEnabled: true
            anchors.fill: parent
            ToolTip.delay: 500
            ToolTip.text: qsTr("Contract")
            ToolTip.visible: containsMouse
            onClicked: mainWindow.exitFullScreen()
            activeFocusOnTab: true
            Keys.onPressed: {
                if (event.key === Qt.Key_Enter || event.key === Qt.Key_Return) {
                    mainWindow.exitFullScreen();
                }
            }
        }
    }
}
