import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.lingmoui as LingmoUI
import QtQuick.Controls.Material
import org.kde.ocean.impl as Impl

Rectangle {
    Layout.fillWidth: true

    default property alias content : _mainLayout.data
    property alias spacing: _mainLayout.spacing
    property alias layout: _mainLayout

    LingmoUI.Theme.colorSet: LingmoUI.Theme.Window
    LingmoUI.Theme.inherit: true

    // color: LingmoUI.Theme.backgroundColor
    radius: 16
    color: root.placeholderTextColor

    // Behavior on color {
    //     ColorAnimation {
    //         duration: 200
    //         easing.type: Easing.Linear
    //     }
    // }

    implicitHeight: _mainLayout.implicitHeight +
                    _mainLayout.anchors.topMargin +
                    _mainLayout.anchors.bottomMargin

    ColumnLayout {
        id: _mainLayout
        anchors.fill: parent
        anchors.leftMargin: LingmoUI.Units.largeSpacing * 1.5
        anchors.rightMargin: LingmoUI.Units.largeSpacing * 1.5
        anchors.topMargin: LingmoUI.Units.largeSpacing
        anchors.bottomMargin: LingmoUI.Units.largeSpacing
        spacing: LingmoUI.Units.largeSpacing
    }
}
