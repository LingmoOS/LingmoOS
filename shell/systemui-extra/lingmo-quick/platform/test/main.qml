import QtQuick 2.15
import QtQuick.Controls 1.4

Rectangle {
    TabView {
        width: parent.width
        Tab {
            width: parent.width
            title: "Palette"
            PaletteColorPage {}
        }
        Tab {
            width: parent.width
            title: "Settings"
            SettingsValuePage {}
        }
        Tab {
            width: parent.width
            title: "AppLauncher"
            AppLauncherPage {}
        }
        Tab {
            width: parent.width
            title: "ToolTip"
            ToolTipTest {}
        }
    }
}