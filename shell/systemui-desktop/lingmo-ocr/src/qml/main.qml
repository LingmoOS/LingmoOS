import QtQuick 2.4
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.3
import QtQuick.Window 2.3
import QtGraphicalEffects 1.0
import QtQuick.Dialogs 1.2
import LingmoUI 1.0 as LingmoUI
import Lingmo.Ocr 1.0
import "../"
LingmoUI.Window {
    id: rootWindow
    title: qsTr("Ocr")
    visible: true
    width: 600
    height: 500

    minimumWidth: 600
    minimumHeight: 500

    header.height: 40
    contentTopMargin: 0

    LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft
    LayoutMirroring.childrenInherit: true

    background.color: LingmoUI.Theme.secondBackgroundColor

    Ocr {
        id: ocr
    }
    headerItem: Item {
        Row{
            anchors.fill: parent
            anchors.leftMargin: LingmoUI.Units.smallSpacing * 1.5
            anchors.rightMargin: LingmoUI.Units.smallSpacing * 1.5
            anchors.topMargin: LingmoUI.Units.smallSpacing * 1.5
            anchors.bottomMargin: LingmoUI.Units.smallSpacing * 1.5

            spacing: LingmoUI.Units.smallSpacing
            ComboBox {
                id:type
                width: 100
                height: 30
                padding: 0
                model: ["简体中文","繁体中文","English"]
                topInset: 0
                bottomInset: 0
                currentIndex: 0
                onActivated: {
                    ocr.setTextType(currentText)
                }
            }
            Button{
                icon.source: LingmoUI.Theme.darkMode ? "qrc:/assets/open_light.svg" : "qrc:/assets/open_dark.svg"
                icon.width: height
                icon.height: height
                width: height
                height: 30
                onClicked: fileDialog.open()
            }
            Button{
                icon.source: LingmoUI.Theme.darkMode ? "qrc:/assets/copy_light.svg" : "qrc:/assets/copy_dark.svg"
                icon.width: height
                icon.height: height
                width: height
                height: 30
                onClicked: ocr.copy()
            }
        }
    }


        Rectangle {
            anchors.fill: parent
            anchors.topMargin: rootWindow.header.height + LingmoUI.Units.smallSpacing
            color: LingmoUI.Theme.backgroundColor
            radius: LingmoUI.Theme.bigRadius
            DropArea {
                id: drop_area
                anchors.fill: parent
                onDropped: (drop)=>{
                    console.log("drop hasUrls:", drop.hasUrls,drop.urls[0])
                    if (drop.hasUrls) {
                        ocr.openImage(drop.urls[0])
                    }
                }
            }
            FileDialog {
                id: fileDialog
                title: tr("Please choose a img")

                folder: shortcuts.home
                onAccepted: {
                    console.log("You chose: " + fileDialog.fileUrls)
                    ocr.openImage(fileDialog.fileUrls)
                }
                onRejected: {
                    console.log("Canceled")
                }
            }
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: LingmoUI.Units.smallSpacing * 1.5
                spacing: 0
            RowLayout{
                Layout.fillHeight: true
                Layout.fillWidth: true

                    Item{
                        id: mapItemArea
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                        clip: true
                        Image {
                            id: mapImg
                            x: mapItemArea.width/2-mapImg.width/2
                            y: mapItemArea.height/2-mapImg.height/2
                            source: ocr.imgName
                            //图像异步加载，只对本地图像有用
                            asynchronous: true
                        }
                        MouseArea {
                            id: mapDragArea
                            anchors.fill: mapImg
                            drag.target: mapImg
                            //这里使图片不管是比显示框大还是比显示框小都不会被拖拽出显示区域
                            drag.minimumX: (mapImg.width > mapItemArea.width) ? (mapItemArea.width - mapImg.width) : 0
                            drag.minimumY: (mapImg.height > mapItemArea.height) ? (mapItemArea.height - mapImg.height) : 0
                            drag.maximumX: (mapImg.width > mapItemArea.width) ? 0 : (mapItemArea.width - mapImg.width)
                            drag.maximumY: (mapImg.height > mapItemArea.height) ? 0 : (mapItemArea.height - mapImg.height)

                            //使用鼠标滚轮缩放
                            onWheel: {
                                //每次滚动都是120的倍数
                                var datla = wheel.angleDelta.y/120;
                                if(datla > 0)
                                {
                                    mapImg.scale = mapImg.scale/0.9
                                }
                                else
                                {
                                    mapImg.scale = mapImg.scale*0.9
                                }
                            }
                        }
                    }

                    Item{
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                        ScrollView {
                            id: scView
                            anchors.fill: parent

                            background: Rectangle {
                                anchors.fill: parent
                                color: LingmoUI.Theme.secondBackgroundColor
                                radius: LingmoUI.Theme.bigRadius
                            }

                            TextArea {
                                id: contentText
                                wrapMode: TextArea.WrapAnywhere
                                font.pixelSize: 14
                                selectByMouse: true
                                text: ocr.ocrText
                                color: LingmoUI.Theme.textColor
                            }
                        }
                    }
                }
        }

        }
    function openImage(name){
        ocr.openImage(name)
    }

    onClosing: function(closeevent){
        Qt.exit(0)
    }
}
