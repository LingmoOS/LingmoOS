import QtQuick 2.12
import QtQuick.Layouts 1.12
import org.lingmo.menu.core 1.0
import org.lingmo.quick.platform 1.0 as Platform
import org.lingmo.quick.items 1.0 as LingmoUIItems

FocusScope {
    id: normalUI

    Component.onCompleted: {
        forceFocus();
        mainWindow.visibleChanged.connect(forceFocus);
    }
    Component.onDestruction: {
        mainWindow.visibleChanged.disconnect(forceFocus);
    }

    function forceFocus() {
        normalUI.focus = true;
        if (mainWindow.visible) {
            appPage.search.changeToSearch("");
        }
    }
    function keyPressed(event) {
        // 任意字符键焦点切换到搜索（0-9 a-z）
        if ((0x2f < event.key && event.key < 0x3a)||(0x40 < event.key && event.key < 0x5b)) {
            focus = true;
            appPage.search.changeToSearch(event.text);
            // 任意方向键切换至应用列表
        } else if ((0x01000011 < event.key)&&(event.key < 0x01000016)) {
            focus = true;
            appPage.content.focus = true;
            appPage.content.resetFocus();
        }
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0
        z: 10

        AppPage {
            id: appPage
            Layout.preferredWidth: 312
            Layout.fillHeight: true
        }

        LingmoUIItems.StyleBackground {
            Layout.preferredWidth: 1
            Layout.fillHeight: true

            paletteRole: Platform.Theme.ButtonText
            useStyleTransparency: false
            alpha: 0.05
        }

        WidgetPage {
            Layout.fillWidth: true
            Layout.fillHeight: true
        }

        LingmoUIItems.StyleBackground {
            Layout.preferredWidth: 1
            Layout.fillHeight: true

            paletteRole: Platform.Theme.ButtonText
            useStyleTransparency: false
            alpha: 0.05
        }

        Sidebar {
            Layout.preferredWidth: 60
            Layout.fillHeight: true
        }
    }
}
