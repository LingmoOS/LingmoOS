// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.0
import QtQml 2.11
import org.deepin.dtk 1.0
import ".."

Column {
    id: panel
    Label {
        width: parent.width
        wrapMode: Text.WordWrap
        text: "菜单控件，主要出现在右键，DCombobox弹出，主菜单，搜索框的补全等一些地方。带尖角的菜单有明确的指向，告诉用户这个菜单对应的是哪个地方的。"
        horizontalAlignment: Qt.AlignHCenter
    }

    spacing: 10

    Item {
        visible: true
        width: parent.width
        height: 1050

        ImageCell {
            x: 0; y: 0
            source: "qrc:/assets/menu/rightclick.svg"
            Menu {
                MenuItem { text: qsTr("打开") }
                MenuItem { text: qsTr("在新窗口打开") }
                MenuItem { text: qsTr("在新标签中打开") }
                MenuItem {
                    text: qsTr("以管理员身份打开")
                    enabled : false
                }
                MenuSeparator {}
                MenuItem { text: qsTr("复制") }
                MenuSeparator {}
                MenuItem { text: qsTr("共享文件夹")}
                MenuItem { text: qsTr("创建链接")}
                MenuItem { text: qsTr("发送到桌面")}
                MenuItem { text: qsTr("在终端中打开")}
                Menu { title: qsTr("自定义")}
                MenuItem { text: qsTr("压缩")}
                MenuItem { text: qsTr("属性")}
            }
        }

        ImageCell {
            x: 230; y: 0
            source: "qrc:/assets/menu/music.svg"
            Menu {
                MenuSeparator { text: qsTr("单曲")}
                MenuItem { icon.name: "folder-music-symbolic"; text: qsTr("Tomorrow-Fly By Midnight") }
                MenuItem { icon.name: "folder-music-symbolic"; text: qsTr("Tomorrow With You (EN]ABA)- Senpai Kondor") }
                MenuItem { icon.name: "folder-music-symbolic"; text: qsTr("Tomorrow will be fine. - Sodagreen") }
                MenuItem { icon.name: "folder-music-symbolic"; text: qsTr("CROWN - TOMORROW X TOGETHER") }
                MenuSeparator { text: qsTr("歌手")}
                MenuItem { icon.name: "music"; text: qsTr("Tomorrow - Fly By Midnight") }
                MenuSeparator { text: qsTr("专辑")}
                MenuItem { icon.name: "music"; text: qsTr("DTK-Menu-Menuitem-ICON-Right-Light") }
                MenuItem { icon.name: "music"; text: qsTr("CROWN - TOMORROW X TOGETHER") }
            }
        }

        ImageCell {
            x: 630; y: 0
            width: 340
            source: "qrc:/assets/menu/language.svg"
            Menu {
                id: searchAndArrowMenu
                header: SearchEdit {
                    placeholder: qsTr("搜索")
                    onTextChanged: {
                        proxyModel.filterText = text
                        proxyModel.update()
                    }
                }
                model: ObjectModelProxy {
                    id: proxyModel
                    property string filterText
                    filterAcceptsItem: function(item) {
                        return item.text.includes(filterText)
                    }
                    sourceModel: searchAndArrowMenu.contentModel
                }

                maxVisibleItems: 10
                MenuItem { text: qsTr("Greek(cp869)") }
                MenuItem { text: qsTr("Cyrillic (ISO 8859-5)") }
                MenuItem { text: qsTr("Cyrillic(KOI8-R)") }
                MenuItem { text: qsTr("Devanagari(x-mac-davanagari)") }
                MenuItem { text: qsTr("Gurmukhi(x-mac-gurmukhi)") }
                MenuItem { text: qsTr("Thai (ISO 8859-11)") }
                MenuItem { text: qsTr("Simplified Chinese(x-mac-sim-chinese)") }
                MenuItem { text: qsTr("Chinese(GBK)"); checked: true}
                MenuItem { text: qsTr("Chinese(GB 18030)") }
                MenuItem { text: qsTr("Chinese(ISO 2022-CN)") }
                MenuItem { text: qsTr("Simplified Chinese (GB2312)") }
                MenuItem { text: qsTr("Tibetan (x-mac-tibetan)") }
                MenuItem { text: qsTr("Central European(ISO Latin 2)") }
                MenuItem { text: qsTr("Central Eu 188 opx(ISO Latin 4)") }
                MenuItem { text: qsTr("Baltic (ISO Latin 7)") }
            }
        }

        ImageCell {
            x: 0; y: 400
            source: "qrc:/assets/menu/cancle.svg"
            Menu {
                MenuItem { icon.name: "music"; text: qsTr("撤销") }
                MenuItem { icon.name: "music"; text: qsTr("恢复") }
                MenuSeparator {}
                MenuItem { icon.name: "music"; text: qsTr("剪切") }
                MenuItem { icon.name: "music"; text: qsTr("复制") }
                MenuItem { icon.name: "music"; text: qsTr("粘贴") }
                MenuItem { icon.name: "music"; text: qsTr("删除") }
                MenuSeparator {}
                MenuItem { icon.name: "music"; text: qsTr("全选") }
            }
        }

        ImageCell {
            x: 250; y: 350
            source: "qrc:/assets/menu/new1.svg"
            Menu {
                Menu {
                    title: "新建"
                    width: 150
                    MenuItem { text: ("新建文件夹") }
                    MenuItem { text: "文件" }
                    MenuItem { text: qsTr("办公文件") }
                    MenuItem { text: qsTr("电子表格") }
                    MenuItem { text: qsTr("演示文档") }
                    MenuItem { text: qsTr("文本文档") }
                }
                MenuItem { text: "排序方式" }
                Menu { title: "桌面显示" }
                MenuItem { text: "全选" }
                MenuItem { text: "在终端打开" }
                MenuSeparator {}
                Menu { title: "显示设置" }
                Menu { title: "壁纸与屏保" }
            }
        }
        ImageCell {
            x: 450; y: 350
            source: "qrc:/assets/menu/new1-1.svg"
        }


        ImageCell {
            x: 0; y: 650
            source: "qrc:/assets/menu/font.svg"
            Menu {
                id: fontMenu
                header: SearchEdit {
                    placeholder: qsTr("搜索")
                }

                Instantiator {
                    asynchronous: true
                    model: [
                        "等距更纱黑体 SC",
                        "更纱黑体 SC",
                        "方正黑体_GBK",
                        "方正魏碑_GBK",
                        "方正隶书_GBK",
                        "方正行楷_GBK",
                        "方正细黑一_GBK",
                        "方正楷体_GBK",
                    ]
                    delegate: MenuItem { }
                    onObjectAdded: function(index, object) {
                        object.text = model[index]
                        object.font.family = object.text
                        fontMenu.insertItem(index, object)
                    }
                }
            }
        }

        ImageCell {
            x: 250; y: 600
            source: "qrc:/assets/menu/new2.svg"
            Menu {
                id: subMenuAndCheckMenu

                Menu { title: "新建" }
                Menu {
                    title: qsTr("排序方式")
                    width: 150
                    MenuItem { autoExclusive: true; checkable: true; text: qsTr("名称"); checked: true }
                    MenuItem { autoExclusive: true; checkable: true; text: qsTr("修改时间") }
                    MenuItem { autoExclusive: true; checkable: true; text: qsTr("创建时间") }
                    MenuItem { autoExclusive: true; checkable: true; text: qsTr("大小") }
                    MenuItem { autoExclusive: true; checkable: true; text: qsTr("文件类型") }
                }
                Menu { title: "桌面显示" }
                MenuItem { text: "全选" }
                MenuItem { text: "在终端打开" }
                MenuSeparator {}
                Menu { title: "显示设置" }
                Menu { title: "壁纸与屏保" }
            }
        }
        ImageCell {
            x: 450; y: 600
            source: "qrc:/assets/menu/new2-1.svg"
        }

        ImageCell {
            x: 700; y: 580
            source: "qrc:/assets/menu/voice1.svg"
            Menu {
                MenuItem { checkable: true; icon.name: "music"; text: qsTr("语音记事本") }
                MenuItem { checkable: true; icon.name: "music"; text: qsTr("文本编辑器"); checked: true }
                MenuItem { checkable: true; icon.name: "music"; text: qsTr("WPS Office") }
                MenuSeparator {}
                MenuItem { text: "选择其它"}
            }
        }

        ImageCell {
            x: 700; y: 750
            source: "qrc:/assets/menu/voice2.svg"
            Menu {
                MenuItem { checkable: true; icon.name: "music"; text: qsTr("语音记事本") }
                MenuItem { checkable: true; icon.name: "music"; text: qsTr("文本编辑器") }
                MenuItem { checkable: true; icon.name: "music"; text: qsTr("WPS Office") }
                MenuSeparator {}
                MenuItem { text: "选择其它"}
            }
        }

        ImageCell {
            x: 250; y: 900
            source: "qrc:/assets/menu/projects1.svg"
            Menu {
                MenuItem { checkable: true; icon.name: "music"; text: qsTr("所有项") }
                MenuItem { checkable: true; icon.name: "music"; text: qsTr("照片")}
                MenuItem { checkable: true; icon.name: "music"; text: qsTr("视频") }
            }
        }

        ImageCell {
            x: 490; y: 900
            source: "qrc:/assets/menu/projects2.svg"
            Menu {
                MenuItem { checkable: true; icon.name: "music"; text: qsTr("所有项") }
                MenuItem { checkable: true; icon.name: "music"; text: qsTr("照片"); checked: true }
                MenuItem { checkable: true; icon.name: "music"; text: qsTr("视频") }
            }
        }

        ImageCell {
            x: 730; y: 900
            source: "qrc:/assets/menu/projects3.svg"
            Menu {
                MenuItem { display: IconLabel.IconBesideText; icon.name: "music"; text: qsTr("所有项") }
                MenuItem { display: IconLabel.IconBesideText; icon.name: "music"; text: qsTr("照片") }
                MenuItem { display: IconLabel.IconBesideText; icon.name: "music"; text: qsTr("视频") }
            }
        }
    }
}
