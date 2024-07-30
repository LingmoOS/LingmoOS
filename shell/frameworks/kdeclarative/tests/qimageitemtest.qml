import QtQuick 2.0
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.15
import org.kde.kquickcontrolsaddons 2.0 as KQCA

//this file should be used via qimageitemtest executable
//unless you want to test isNull

Rectangle {
    color: "white"
    width: 500
    height: 500

    RowLayout {
        anchors.fill: parent
        KQCA.QImageItem {
            implicitWidth: 300
            implicitHeight: 300

            id: image
            image: testImage
            fillMode: fillModeCombo.currentIndex
        }
        GridLayout {
            columns: 2
            Text {text: "fillMode"}
            ComboBox {
                id: fillModeCombo
                model: ListModel{
                    ListElement {
                        display: "Stretch"
                    }
                    ListElement {
                        display: "PreserveAspectFit"
                    }ListElement {
                        display: "PreserveAspectCrop"
                    }ListElement {
                        display: "Tile"
                    }ListElement {
                        display: "TileVertically"
                    }
                    ListElement {
                        display: "TileHorizontally"
                    }
                }
            }

            Text {text: "isNull:"}
            Text {text: image.null}
            Text {text: "nativeWidth:"}
            Text {text: image.nativeWidth}
            Text {text: "nativeHeight:"}
            Text {text: image.nativeHeight}
            Text {text: "paintedWidth:"}
            Text {text: image.paintedWidth}
            Text {text: "paintedHeight:"}
            Text {text: image.paintedHeight}

            Text  {text: "rect size is 300x300"}
        }
    }
}
