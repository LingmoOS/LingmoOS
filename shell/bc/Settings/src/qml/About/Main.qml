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
                visible: true
                anchors {
                    top: parent.top
                    bottom: lf.top
                    left: parent.left
                    // horizontalCenter: parent.horizontalCenter
                    leftMargin:80
                    right: parent.right
                    margins: 5 // 设置边距为20像素
                }

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

                // Image {
                //     id: logo
                //     anchors {
                //         verticalCenter: parent.verticalCenter
                //         right: lbt.left
                //         rightMargin: sysinf.width / 30
                //     }
                //     width: 300
                //     sourceSize: Qt.size(width, height)
                //     source: LingmoUI.Theme.darkMode ? "qrc:/images/dark/logo.svg" : "qrc:/images/light/logo.svg"

                //     layer.enabled: true
                //     layer.effect: ShaderEffect {
                //         property variant src: logo
                //         property real shift: 0.0
                //         property bool darkMode: LingmoUI.Theme.darkMode

                //         fragmentShader: "
                //             uniform lowp float qt_Opacity;
                //             uniform sampler2D src;
                //             uniform lowp float shift;
                //             uniform bool darkMode;
                //             varying highp vec2 qt_TexCoord0;

                //             void main() {
                //                 vec4 color = texture2D(src, qt_TexCoord0);

                //                 // 保持透明区域透明
                //                 if (color.a < 0.01) {
                //                     gl_FragColor = color * qt_Opacity;
                //                     return;
                //                 }

                //                 float pos = qt_TexCoord0.x;
                //                 float width = 0.2;
                //                 float factor = smoothstep(0.0, width, abs(pos - shift));

                //                 vec3 finalColor;
                //                 if (darkMode) {
                //                     finalColor = color.rgb * (1.0 - 0.5 * (1.0 - factor));
                //                 } else {
                //                     gl_FragColor = color * qt_Opacity;
                //                 }

                //                 gl_FragColor = vec4(finalColor, color.a) * qt_Opacity;
                //             }
                //         "

                //         NumberAnimation on shift {
                //             from: -0.3
                //             to: 1.3
                //             duration: 2500
                //             loops: Animation.Infinite
                //             running: true
                //         }
                //     }
                // }

                Rectangle {
                    id: lbt
                    
                    anchors {
                        centerIn: parent
                        // verticalCenter: parent.verticalCenter                     
                    }
                    height: 20
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
                    width: logo.width/1.44
                    height: logo.width/2.4
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

            // Item {
            //     id: topArea
            //     Layout.fillWidth: true
            //     height: 200

            //     // 开启图层，并整体裁剪圆角
            //     layer.enabled: true
            //     layer.effect: OpacityMask {
            //         maskSource: Rectangle {
            //             width: topArea.width
            //             height: topArea.height
            //             radius: LingmoUI.Theme.bigRadius
            //             color: "black"   // 必须有颜色，否则 mask 无效
            //         }
            //     }

            //     // 背景图片
            //     Image {
            //         id: bgImage
            //         anchors.fill: parent
            //         source: "file://" + wallpaper.path
            //         fillMode: Image.PreserveAspectCrop
            //     }

            //     // FastBlur 模糊
            //     FastBlur {
            //         anchors.fill: bgImage
            //         source: bgImage
            //         radius: 55
            //     }

            //     // 半透明遮罩
            //     Rectangle {
            //         anchors.fill: parent
            //         color: LingmoUI.Theme.darkMode ? "#66000000" : "#5DFFFFFF"
            //     }

            //     // Logo 居中
            //     Image {
            //         id: systemlogo
            //         anchors.centerIn: parent
            //         width: 300
            //         sourceSize: Qt.size(width, height)
            //         source: LingmoUI.Theme.darkMode ?
            //                     "qrc:/images/dark/logo.svg" :
            //                     "qrc:/images/light/logo.svg"

            //         layer.enabled: true
            //         layer.effect: DropShadow {
            //             horizontalOffset: 0
            //             verticalOffset: 4
            //             radius: 16           // 阴影模糊半径
            //             color: "#D3FFFFFF"   // 阴影颜色，可调透明度
            //             samples: 18          // 阴影质量，值越高越平滑
            //         }
            //     }

            // }

            Item {
                height: LingmoUI.Units.smallSpacing
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
                            // value: qsTr("12 Bookworm")
                            value: about.debianversion
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

                    Image {
                        source: LingmoUI.Theme.darkMode ? "qrc:/images/dark/elinks.svg" : "qrc:/images/light/elinks.svg"
                        width: 12
                        sourceSize: Qt.size(width, height)
                    }
                }
            }

            Item {
                height: LingmoUI.Units.smallSpacing
            }

            StandardButton {
                text: ""
                Layout.fillWidth: true
                visible: about.checkForLingmoOS()
                onClicked: {
                    Qt.openUrlExternally("https://wiki.lingmo.org/wiki/develop-guide")
                }

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: LingmoUI.Units.largeSpacing * 1.5
                    anchors.rightMargin: LingmoUI.Units.largeSpacing * 1.5

                    Label {
                        text: qsTr("Open Source Software Statement")
                    }

                    Item {
                        Layout.fillWidth: true
                    }

                    Image {
                        source: LingmoUI.Theme.darkMode ? "qrc:/images/dark/elinks.svg" : "qrc:/images/light/elinks.svg"
                        width: 12
                        sourceSize: Qt.size(width, height)
                    }
                }
            }
            

            Item {
                height: LingmoUI.Units.smallSpacing
            }
        }
    }
}
