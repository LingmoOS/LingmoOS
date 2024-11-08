import QtQuick 2.15
import QtQml 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import LingmoUI 1.0 as LingmoUI

Item {
    id: control

    property bool checked: false
    property bool hovered: _mouseArea.containsMouse
    property bool pressed: _mouseArea.pressed

    property alias font: _label.font
    property string text: ""

    property var contentWidth: _contentLayout.implicitWidth + LingmoUI.Units.largeSpacing * 2

    property var backgroundColor: LingmoUI.Theme.secondBackgroundColor
    property var hoveredColor: LingmoUI.Theme.darkMode ? Qt.lighter(LingmoUI.Theme.secondBackgroundColor, 1.3)
                                                     : Qt.darker(LingmoUI.Theme.secondBackgroundColor, 1.05)
    property var pressedColor: LingmoUI.Theme.darkMode ? Qt.lighter(LingmoUI.Theme.secondBackgroundColor, 1.1)
                                                     : Qt.darker(LingmoUI.Theme.secondBackgroundColor, 1.1)

    property var highlightColor: LingmoUI.Theme.highlightColor
    property var highlightHoveredColor: Qt.lighter(control.highlightColor, 1.1)
    property var highlightPressedColor: Qt.darker(control.highlightColor, 1.1)

    property alias background: hoveredRect

    signal clicked()
    signal closeClicked()

    MouseArea {
        id: _mouseArea
        anchors.fill: parent
        hoverEnabled: true
        onClicked: control.clicked()
    }

    Rectangle {
        id: hoveredRect
        anchors.fill: parent
        anchors.leftMargin: LingmoUI.Units.smallSpacing / 2
        anchors.rightMargin: LingmoUI.Units.smallSpacing / 2
        anchors.topMargin: LingmoUI.Units.smallSpacing / 2
        color: control.hovered ? control.pressed ? pressedColor
                                                 : hoveredColor : backgroundColor
        opacity: 0.5
        border.width: 0
        radius: LingmoUI.Theme.smallRadius
    }

    Rectangle {
        id: checkedRect
        anchors.leftMargin: LingmoUI.Units.smallSpacing / 2
        anchors.rightMargin: LingmoUI.Units.smallSpacing / 2
        anchors.topMargin: LingmoUI.Units.smallSpacing / 2
        anchors.fill: parent

        color: control.hovered ? control.pressed ? highlightPressedColor
                                                 : highlightHoveredColor : highlightColor

        opacity: _mouseArea.pressed ? 0.9 : 1
        border.width: 0
        visible: checked
        radius: LingmoUI.Theme.smallRadius
    }

    RowLayout {
        id: _contentLayout
        anchors.fill: parent
        anchors.leftMargin: LingmoUI.Units.smallSpacing
        anchors.rightMargin: LingmoUI.Units.smallSpacing
        anchors.topMargin: LingmoUI.Units.smallSpacing / 2
        spacing: 0

        Label {
            id: _label
            text: control.text
            leftPadding: 0
            rightPadding: 0
            Layout.fillWidth: true
            Layout.fillHeight: true
            horizontalAlignment: Qt.AlignHCenter
            verticalAlignment: Qt.AlignVCenter
            color: control.checked ? LingmoUI.Theme.highlightedTextColor
                                   : LingmoUI.Theme.textColor
            elide: Text.ElideMiddle
            wrapMode: Text.NoWrap
        }

        LingmoUI.TabCloseButton {
            id: _closeButton
            enabled: control.checked
            Layout.preferredHeight: 24
            Layout.preferredWidth: 24
            size: 24
            source: !enabled ? "" : "qrc:/images/" + (LingmoUI.Theme.darkMode || control.checked ? "dark/" : "light/") + "close.svg"
            onClicked: control.closeClicked()
        }
    }
}
