import QtQuick 2.6
import QtQuick.Controls 2.3
import QtQuick.Templates 2.3 as T
import LingmoUI 1.0 as LingmoUI

T.Frame {
    id: control

    implicitWidth: contentWidth + leftPadding + rightPadding
    implicitHeight: contentHeight + topPadding + bottomPadding

    contentWidth: contentItem.implicitWidth || (contentChildren.length === 1 ? contentChildren[0].implicitWidth : 0)
    contentHeight: contentItem.implicitHeight || (contentChildren.length === 1 ? contentChildren[0].implicitHeight : 0)

    padding: 6

    background: Rectangle {
        color: "transparent"
        property color borderColor: LingmoUI.Theme.textColor
        border.color: Qt.rgba(borderColor.r, borderColor.g, borderColor.b, 0.3)
    }
}
