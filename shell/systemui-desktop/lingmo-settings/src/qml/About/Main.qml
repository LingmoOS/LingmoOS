import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtGraphicalEffects 1.0
import Lingmo.System 1.0 as System
import LingmoUI 1.0 as LingmoUI
import Lingmo.Settings 1.0
import "../"

ItemPage {
    id: control
    headerTitle: qsTr("About")

    About {
        id: about
        signal hostNameChanged(string newHostName)
    }

    System.Wallpaper {
        id: wallpaper
    }

    Scrollable {
        anchors.fill: parent
        contentHeight: layout.implicitHeight

        ColumnLayout {
            id: layout
            anchors.fill: parent

            SysItem {
                id: sysinf

                Image {
                    id: logo
                    anchors {
                        // top: parent.top
                        // bottom: parent.bottom
                        verticalCenter: parent.verticalCenter
                        right: lbt.left
                        rightMargin: sysinf.width/30
                    }
                    width: 250
                    sourceSize: Qt.size(width, height)
                    source: LingmoUI.Theme.darkMode ? "qrc:/images/logo-2.svg" : "qrc:/images/logo-2.svg"
                }

                Rectangle {
                    id: lbt
                    anchors {
                        centerIn: parent
                        // verticalCenter: parent.verticalCenter                     
                    }
                    height: sysinf.height/1.2
                    width: 2
                    color: LingmoUI.Theme.disabledTextColor
                    opacity: LingmoUI.Theme.darkMode ? 0.5 : 0.5
                }

                Rectangle {
                    id: deviceItem

                    anchors {
                        left: lbt.right
                        verticalCenter: parent.verticalCenter
                        leftMargin: sysinf.width/30
                    }
                    // width: logo.width/1.2
                    // height: logo.width/2
                    width: logo.width/1.2
                    height: logo.width/2
                    color: LingmoUI.Theme.backgroundColor
                    border.width: 5
                    border.color: LingmoUI.Theme.textColor
                    radius: LingmoUI.Theme.bigRadius

                    Image {
                        id: _image
                        width: deviceItem.width - 6
                        height: deviceItem.height - 6
                        anchors {
                            verticalCenter: parent.verticalCenter
                            horizontalCenter: parent.horizontalCenter
                        }
                        // source: LingmoUI.Theme.darkMode ? "qrc:/images/MundoDark.jpeg" : "qrc:/images/MundoLight.jpeg"
                        source: "file://" + wallpaper.path
                        sourceSize: Qt.size(width, height)
                        fillMode: Image.PreserveAspectCrop
                        asynchronous: true
                        mipmap: true
                        cache: true
                        smooth: true
                        opacity: 1.0

                        Behavior on opacity {
                            NumberAnimation {
                                duration: 100
                                easing.type: Easing.InOutCubic
                            }
                        }

                        layer.enabled: true
                        layer.effect: OpacityMask {
                            maskSource: Item {
                                width: _image.width
                                height: _image.height
                                Rectangle {
                                    anchors.fill: parent
                                    radius: LingmoUI.Theme.bigRadius
                                }
                            }
                        }
                    }

                    Loader {
                        id: bgLoader
                        anchors.fill: parent

                        sourceComponent: {
                            if (background.backgroundType === 0)
                                return wallpaperItem

                            return colorItem
                        }
                    }
                }
            }

            // RoundedItem {
            //     StandardItem {
            //         key: qsTr("OS Version")
            //         // key: qsTr("System Version")
            //         value: about.version
            //     }

            //     Horizontalabt {}

            //     StandardItem {
            //         key: qsTr("System Release")
            //         value: about.systemrelease
            //     }
            // }

            RoundedItem {
                id: lf
                PCname {
                    id: pcName
                    key: qsTr("PC Name")
                    value: about.hostName
                }

                Horizontalabt {}

                StandardItem {
                    key: qsTr("OS Version")
                    value: about.version
                }
            }

            Item {
                height: LingmoUI.Units.smallSpacing
            }

            RoundedItem {
                id: upItem

                Rectangle {
                    id: recup
                    anchors.fill: parent
                    color: "transparent"
                    radius: LingmoUI.Theme.smallRadius
                }

                Grid {
                    Layout.fillWidth: true
                    spacing: 5
                    rows: 1
                    columns: 4
                    // Rectangle {
                    //     id: display
                    //     width: upItem.width/4
                    //     height: 50
                    //     color: "transparent"
                        
                    //     InfoItem {
                            
                    //         key: qsTr("Display")
                    //         value: about.displayInfo
                    //     }
                    // }

                    Rectangle {
                        id: debianversion
                        width: upItem.width/4
                        height: 50
                        color: "transparent"
                        InfoItem {
                            key: qsTr("Debian Version")
                            value: qsTr("13 Trixie")
                        }
                    }
                    Rectangle {
                        id: kernel
                        width: upItem.width/4
                        height: 50
                        color: "transparent"
                        InfoItem {
                            key: qsTr("Linux Kernel Version")
                            value: qsTr("Linux ") + about.kernelVersion
                        }
                    }
                    Rectangle {
                        id: uiversion
                        width: upItem.width/4
                        height: 50
                        color: "transparent"
                        InfoItem {
                            key: qsTr("Lingmo UI Version")
                            value: about.uiversion
                        }
                    }
                    
                    Rectangle {
                        id: systype
                        width: upItem.width/4
                        height: 50
                        color: "transparent"
                        InfoItem {
                            key: qsTr("System Type")
                            value: about.architecture
                        }
                    }
                }

                Horizontalabt {}

                Rectangle {
                    id: recdown
                    anchors.fill: parent
                    color: "transparent"
                    radius: LingmoUI.Theme.smallRadius
                }

                Grid {
                    Layout.fillWidth: true
                    spacing: 3
                    rows: 1
                    columns: 4
                    
                    Rectangle {
                        id: ram
                        width: upItem.width/4
                        height: 50
                        color: "transparent"
                        InfoItem {
                            key: qsTr("RAM")
                            value: about.memorySize
                        }
                    }
                    Rectangle {
                        id: disk
                        width: upItem.width/4
                        height: 50
                        color: "transparent"
                        InfoItem {
                            key: qsTr("Disk")
                            value: about.internalStorage
                        }
                    }

                    Rectangle {
                        id: cpu
                        width: upItem.width/4
                        height: 50
                        color: "transparent"
                        InfoItem {
                            key: qsTr("CPU")
                            value: about.cpuInfo
                        }
                    }
                    // Rectangle {
                    //     id: merit
                    //     width: upItem.width/4
                    //     height: 50
                    //     color: "transparent"
                    //     InfoItem {
                    //         key: qsTr("Lingmo OS Merit Wall")
                    //         value: qsTr("Click here to visit")
                    //     }
                    // }
                }
            }

            Item {
                height: LingmoUI.Units.smallSpacing
            }

            RoundedItem {
                
                StandardButton {
                    text: ""
                    Layout.fillWidth: true
                    visible: about.checkForLingmoOS()
                    onClicked: {
                        about.openUpdator()
                    }

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: LingmoUI.Units.largeSpacing * 1.5
                        anchors.rightMargin: LingmoUI.Units.largeSpacing * 1.5

                        Label {
                            text: qsTr("System Update")
                        }

                        Item {
                            Layout.fillWidth: true
                        }

                        Label {
                            text: qsTr("→")
                        }
                    }
                }

                Horizontalabt {}

                StandardItem {
                    key: qsTr("Open Source Software Statement")
                }

            }

            // RoundedItem {
            //     StandardItem {
            //         key: qsTr("Desktop Version")
            //         value: about.desktopversion
            //     }

            //     Horizontalabt {}

            //     StandardItem {
            //         key: qsTr("UI Version")
            //         value: about.uiversion
            //     }

            // }

            // Item {
            //     height: LingmoUI.Units.smallSpacing
            // }

            // RoundedItem {
            //     StandardItem {
            //         key: qsTr("System Type")
            //         value: about.architecture
            //     }

            //     Horizontalabt {
                    
            //     }

            //     StandardItem {
            //         key: qsTr("Kernel Version")
            //         value: about.kernelVersion
            //     }

            //     Horizontalabt {

            //     }

            //     StandardItem {
            //         key: qsTr("Processor")
            //         value: about.cpuInfo
            //     }

            //     Horizontalabt {

            //     }

            //     StandardItem {
            //         key: qsTr("RAM")
            //         value: about.memorySize
            //     }

            //     Horizontalabt {

            //     }

            //     StandardItem {
            //         key: qsTr("Internal Storage")
            //         value: about.internalStorage
            //     }
            // }

            Item {
                height: LingmoUI.Units.smallSpacing
            }
        }
    }
}
