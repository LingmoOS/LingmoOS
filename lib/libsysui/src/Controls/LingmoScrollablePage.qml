// 一个带有滚动条的页面组件，适合一个有很多内容需要滚动支持的页面
import QtQuick
import QtQuick.Layouts
import QtQuick.Window
import QtQuick.Controls
import LingmoUI

LingmoPage {
    property bool autoResetScroll: false
    default property alias content: container.data

    Flickable{
        id: flickable
        clip: true
        anchors.fill: parent
        ScrollBar.vertical: LingmoScrollBar {}
        boundsBehavior: Flickable.StopAtBounds
        contentHeight: container.height
        ColumnLayout{
            id:container
            width: parent.width
        }
    }

    function resetScroll() {
        flickable.contentY = 0;
    }

    StackView.onActivated: {
        if (autoResetScroll) {
            resetScroll(); // Call this function to reset the scroll position to the top
        }
    }
}
