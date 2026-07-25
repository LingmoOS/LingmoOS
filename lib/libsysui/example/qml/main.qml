import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LingmoUI

LingmoObject {
    id: root

    // Store the hotkeys object for use in the application
    property alias hotkeys: hotkey_object_list

    Component.onCompleted: {
        LingmoApp.init(root, Qt.locale("zh_CN"));
        LingmoApp.windowIcon = "qrc:/image/logo.svg"
        LingmoTheme.animationEnabled = true;
        LingmoTheme.blurBehindWindowEnabled = true;
        LingmoTheme.darkMode = LingmoThemeType.Dark;

        LingmoRouter.routes = {
            "/": "qrc:/qml/windows/MainWindow.qml",
            "/hotkey": "qrc:/qml/windows/ShortcutWindow.qml"
        };

        LingmoRouter.navigate("/");
    }

    LingmoObject {
        id: hotkey_object_list
         LingmoHotkey{
            name: qsTr("Quit")
            sequence: "Ctrl+Alt+Q"
            onActivated: {
                LingmoRouter.exit()
            }
        }
        LingmoHotkey{
            name: qsTr("Test1")
            sequence: "Alt+A"
            onActivated: {
                LingmoRouter.navigate("/hotkey",{sequence:sequence})
            }
        }
    }
}
