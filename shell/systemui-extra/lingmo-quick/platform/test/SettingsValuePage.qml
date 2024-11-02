import QtQuick 2.0
import org.lingmo.quick.platform 1.0
import org.lingmo.quick.items 1.0
Column {
    spacing: 10
    SettingsValue {
        name: "liteAnimation"
        value: Settings.liteAnimation
    }
    SettingsValue {
        name: "liteFunction"
        value: Settings.liteFunction
    }
    SettingsValue {
        name: "tabletMode"
        value: Settings.tabletMode
    }
}