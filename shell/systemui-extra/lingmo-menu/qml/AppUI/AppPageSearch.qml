import QtQuick 2.12
import QtQuick.Controls 2.5
import org.lingmo.menu.core 1.0
import org.lingmo.quick.platform 1.0 as Platform
import org.lingmo.quick.items 1.0 as LingmoUIItems
import org.lingmo.menu.utils 1.0

Item {
    property Item focusToPageContent
    property bool inputStatus: false
    property alias searchInputBar: searchInputBar

    SearchInputBar {
        id: searchInputBar
        property string providerId: "search"

        anchors.fill: parent
        anchors.leftMargin: 16
        anchors.rightMargin: 16
        radius: Platform.Theme.normalRadius
        changeFocusTarget: focusToPageContent
        visible: true
        onTextChanged: {
            if (text === "") {
                AppPageBackend.group = PluginGroup.Display;
                inputStatus = false;
            } else {
                AppPageBackend.group = PluginGroup.Search;
                AppPageBackend.startSearch(text);
                inputStatus = true;
            }
        }

        Component.onCompleted: {
            text = root.currentSearchText;
        }
    }

    function changeToSearch(keyEvent) {
//        if (header.content === null) {
            searchInputBar.text = keyEvent;
            searchInputBar.textInputFocus();
//        }
    }
}
