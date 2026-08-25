import QtQuick 2.4
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.3
import LingmoUI 1.0 as LingmoUI

Page {
    id: page

    property string headerTitle

    background: Rectangle {
        color: rootWindow.background.color
    }

    header: Item {
        height: rootWindow.header.height

        Label {
            anchors.left: parent.left
            leftPadding: LingmoUI.Units.largeSpacing * 3
            rightPadding: Qt.application.layoutDirection === Qt.RightToLeft ? LingmoUI.Units.largeSpacing * 3 : 0
            topPadding: LingmoUI.Units.largeSpacing
            bottomPadding: 0
            font.pointSize: 12
            text: page.headerTitle
            color: rootWindow.active ? LingmoUI.Theme.textColor : LingmoUI.Theme.disabledTextColor
        }
    }
}
