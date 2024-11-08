import QtQuick 2.9
import QtQuick.Controls 2.0
Item {
    id: root
    property alias containsMouse: mouseArea.containsMouse
    signal posChange(int xOffset, int yOffset)
    implicitWidth: 12   //这里隐式的宽为12
    implicitHeight: 12  //这里隐式的高为12
    property int posType: Qt.ArrowCursor
    //5.10之前, qml是不能定义枚举的，用只读的int属性代替一下。
    readonly property int posLeftTop: Qt.SizeFDiagCursor
    readonly property int posLeftBottom: Qt.SizeBDiagCursor
    readonly property int posRightTop: Qt.SizeBDiagCursor
    readonly property int posRightBottom: Qt.SizeFDiagCursor
    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        property int lastX: 0
        property int lastY: 0
        onContainsMouseChanged: {
            if (containsMouse) {
                cursorShape = posType;
            } else {
                cursorShape = Qt.ArrowCursor;
            }
        }
        onPressedChanged: {
            if (containsPress) {
                lastX = mouseX;
                lastY = mouseY;
            }
        }
        onPositionChanged: {
            if (pressed) {
                posChange(mouseX - lastX, mouseY - lastY)
            }
        }
    }
}
