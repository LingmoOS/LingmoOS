import QtQuick
import QtQuick.Layouts
import org.kde.lingmoui as LingmoUI

LingmoUI.FlexColumn {
    Rectangle {
        color: "red"

        Layout.preferredHeight: 200
        Layout.fillWidth: true
    }
    Rectangle {
        color: "orange"

        Layout.preferredHeight: 100
        Layout.fillWidth: true
    }
    Rectangle {
        color: "yellow"

        Layout.preferredHeight: 50
        Layout.fillWidth: true
    }
    Rectangle {
        color: "green"

        Layout.preferredHeight: 25
        Layout.fillWidth: true
    }
}