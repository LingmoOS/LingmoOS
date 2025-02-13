import QtQuick 2.5
import QtQuick.Layouts 1.1
 
ColumnLayout {
    id: rootElement
    property bool isOpen: false
    property string title: ""
    property alias color: accordionHeader.color
    property alias indicatRectColor: indicatRect.color
    default property alias accordionContent: contentPlaceholder.data
    spacing: 0
 
    Layout.fillWidth: true;
 
    Rectangle {
        id: accordionHeader
        color: "red"
        Layout.alignment: Qt.AlignTop
        Layout.fillWidth: true;
        height: 48
 
        Rectangle{
            id:indicatRect
           x: 16; y: 20
           width: 8; height: 8
           radius: 8
           color: "white"
        }
 
        Text {
            x:34;y:13
            font.family: "Ubuntu"
            font.pixelSize: 14
            color: "#FFFFFF"
            text: rootElement.title
        }
        
        Image {
            y:20
            anchors.right:  parent.right
            anchors.rightMargin: 20
            width: 6; height: 10
            id: indicatImg
            source: "qrc:/collapse_normal.png"
        }
        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: {
                rootElement.isOpen = !rootElement.isOpen
                if(rootElement.isOpen)
                {
                    indicatImg.source = "qrc:/expand_normal.png"
                    indicatImg.width = 10; indicatImg.height = 6
                }else{
                    indicatImg.source = "qrc:/collapse_normal.png"
                    indicatImg.width = 6; indicatImg.height = 10
                }
            }
        }
    }
 
    // This will get filled with the content
    ColumnLayout {
        id: contentPlaceholder
        visible: rootElement.isOpen
        Layout.fillWidth: true;
    }
}