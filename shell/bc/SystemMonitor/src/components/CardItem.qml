import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import LingmoUI 1.0 as LingmoUI

Rectangle {
    id: control

    default property alias content: _content.data
    property string title: ""
    property int leftPadding: LingmoUI.Units.largeSpacing
    property int rightPadding: LingmoUI.Units.largeSpacing
    property int topPadding: LingmoUI.Units.largeSpacing
    property int bottomPadding: LingmoUI.Units.largeSpacing

    color: LingmoUI.Theme.darkMode ? "#363636" : "#FFFFFF"
    radius: LingmoUI.Theme.mediumRadius
    opacity: 0.9

    Rectangle {
        anchors.fill: parent
        color: LingmoUI.Theme.secondBackgroundColor
        radius: parent.radius
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: LingmoUI.Units.smallSpacing

        Label {
            text: control.title
            visible: control.title
            font.bold: true
            Layout.margins: LingmoUI.Units.largeSpacing
        }

        Item {
            id: _content
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.leftMargin: control.leftPadding
            Layout.rightMargin: control.rightPadding
            Layout.topMargin: control.title ? 0 : control.topPadding
            Layout.bottomMargin: control.bottomPadding
        }
    }
} 