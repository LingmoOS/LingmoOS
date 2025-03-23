// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.0
import QtQuick.Layouts 1.11
import QtQuick.Window 2.11
import QtQml.Models 2.11
import org.deepin.dtk 1.0
import ".."

Column {
    id: control
    Label {
        width: parent.width
        wrapMode: Text.WordWrap
        text: "弹窗类控件，包括弹出菜单、弹出窗口、对话框等。"
        horizontalAlignment: Qt.AlignHCenter
    }

    spacing: 10

    Flow {
        spacing: 10
        width: parent.width

        ImageCell {
            source: "qrc:/assets/popup/popup-menu-1.svg"
            Popup {
                width: 200; height: 280
                ColumnLayout {
                    width: parent.width
                    Label {
                        text: "勿扰模式"
                        Layout.alignment: Qt.AlignHCenter
                        font: DTK.fontManager.t5
                    }
                    Switch {
                        checked: true
                        Layout.alignment: Qt.AlignHCenter
                    }
                    Label {
                        text: {
                            if (popupListView.currentItem.currentIndex < 0)
                                return ""
                            var item = popupListView.model.get(popupListView.currentItem.currentIndex)
                            switch (item.type) {
                            case 1:
                                return "今晚7: 00前保持开启"
                            }
                            return ""
                        }
                        Layout.alignment: Qt.AlignHCenter
                        font: DTK.fontManager.t7
                    }
                    ListView {
                        id: popupListView
                        Layout.preferredHeight: contentHeight
                        spacing: 10
                        model: ListModel {
                            ListElement { type: 1; icon: "music"; text: "1小时"}
                            ListElement { type: 2; icon: "music"; text: "直至今晚"}
                            ListElement { type: 3; icon: "music"; text: "直至明早"}
                            ListElement { type: 4; icon: "music"; text: "保持开启"}
                        }
                        delegate: ItemDelegate {
                            width: 180
                            icon.name: model.icon
                            text: model.text
                            checked: index === 1
                        }
                    }
                }
            }
        }

        ImageCell {
            source: "qrc:/assets/popup/popup-menu-2.svg"
            Popup {
                width: 200; height: 370
                ColumnLayout {
                    width: parent.width
                    Label {
                        text: "蓝牙"
                        Layout.alignment: Qt.AlignHCenter
                        font: DTK.fontManager.t5
                    }
                    Switch {
                        checked: true
                        Layout.alignment: Qt.AlignHCenter
                    }

                    MenuSeparator { text: "我的设备"}
                    ListView {
                        Layout.preferredHeight: contentHeight
                        Layout.preferredWidth: 180
                        Layout.alignment: Qt.AlignHCenter
                        spacing: 10
                        model: ObjectModel {
                            ItemDelegate {width: 180; text: "WH-1000MX3"; icon.name: "music"; checked: false }
                            ItemDelegate {width: 180; text: "我的音响"; icon.name: "music"; checked: true }
                            ItemDelegate {width: 180; text: "蓝牙鼠标"; icon.name: "music"; checked: false }
                        }
                    }

                    MenuSeparator { text: "其它设备"}
                    ListView {
                        Layout.preferredHeight: contentHeight
                        Layout.preferredWidth: 180
                        Layout.alignment: Qt.AlignHCenter
                        spacing: 10
                        model: ObjectModel {
                            ItemDelegate {width: 180; text: "SHULE's PC"; icon.name: "music" }
                            ItemDelegate {width: 180; text: "打印机"; icon.name: "music" }
                        }
                    }
                }
            }
        }

        ImageCell {
            source: "qrc:/assets/popup/popup-menu-3.svg"
            Popup {
                width: 280; height: 250
                ColumnLayout {
                    width: parent.width
                    Label {
                        text: "声音"
                        Layout.alignment: Qt.AlignHCenter
                        font: DTK.fontManager.t5
                    }
                    RowLayout {
                        QtIcon { name: "irc-voice" }

                        Slider {
                            Layout.alignment: Qt.AlignHCenter
                            highlightedPassedGroove: true
                            Layout.fillWidth: true
                            from: 0
                            to: 100
                            value: 50
                        }

                        QtIcon { name: "irc-voice" }
                    }

                    MenuSeparator { text: "输出"}
                    ListView {
                        Layout.preferredHeight: contentHeight
                        Layout.preferredWidth: 250
                        Layout.alignment: Qt.AlignHCenter
                        spacing: 10
                        model: ObjectModel {
                            ItemDelegate {width: 250; text: "扬声器"; icon.name: "music"; checked: true }
                            ItemDelegate {width: 250; text: "WH-1000MX3"; icon.name: "music"; checked: false }
                            ItemDelegate {width: 250; text: "声音设置"; icon.name: "music"; checked: false }
                        }
                    }
                }
            }
        }

        ImageCell {
            source: "qrc:/assets/popup/popup-menu-4.svg"
            Popup {
                width: 250; height: 530
                ColumnLayout {
                    width: parent.width
                    Label {
                        text: "蓝牙"
                        Layout.alignment: Qt.AlignHCenter
                        font: DTK.fontManager.t5
                    }
                    Switch {
                        checked: true
                        Layout.alignment: Qt.AlignHCenter
                    }

                    ButtonGroup {
                        id: networkGroup
                    }
                    ListModel {
                        id: networkModel
                        ListElement { type: 1; signalLevel: 1; text: "Uniontech"}
                        ListElement { type: 1; signalLevel: 2; text: "TP-link_1723"}
                        ListElement { type: 2; signalLevel: 1; text: "舒乐的热点"}
                        ListElement { type: 2; signalLevel: 1; text: "花花小仙女WIFI"}
                        ListElement { type: 2; signalLevel: 1; text: "有本事你来蹭啊"}
                        ListElement { type: 2; signalLevel: 2; text: "Uniontech-M"}
                        ListElement { type: 2; signalLevel: 2; text: "Miwifi"}
                        ListElement { type: 2; signalLevel: 2; text: "Guest"}
                        ListElement { type: 2; signalLevel: 2; text: "网络设置"}
                    }

                    MenuSeparator { text: "我的网络"}
                    ListView {
                        Layout.preferredHeight: contentHeight
                        Layout.preferredWidth: 220
                        Layout.alignment: Qt.AlignHCenter
                        spacing: 10
                        model: SortFilterModel {
                            model: networkModel
                            delegate: ItemDelegate {
                                width: 220
                                icon.name: {
                                    if (model.signalLevel === 1)
                                        return "music"
                                    return "irc-voice"
                                }
                                text: model.text
                                checked: index === 1
                                ButtonGroup.group: networkGroup
                            }
                            filterAcceptsItem: function(item) {
                                return item.type === 1
                            }
                            lessThan: null
                        }
                    }

                    ItemDelegate {
                        Layout.fillWidth: true; Layout.preferredHeight: 24;
                        topPadding: 0
                        bottomPadding: 0
                        text: "其它网络";
                        icon.name: networkView.visible ? "go-up": "go-down"
                        display: IconLabel.IconBesideText
                        checkable: false
                        font: DTK.fontManager.t8
                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                networkView.visible = !networkView.visible
                            }
                        }
                    }

                    ListView {
                        id: networkView
                        Layout.preferredHeight: contentHeight
                        Layout.preferredWidth: 220
                        Layout.alignment: Qt.AlignHCenter
                        spacing: 2
                        model: SortFilterModel {
                            model: networkModel

                            delegate: ItemDelegate {
                                width: 220
                                icon.name: {
                                    if (model.signalLevel === 1)
                                        return "music"
                                    return "irc-voice"
                                }
                                text: model.text
                                corners: getCornersForBackground(DelegateModel.visibleIndex, ListView.view.count)
                                ButtonGroup.group: networkGroup
                            }
                            filterAcceptsItem: function(item) {
                                return item.type === 2
                            }
                            lessThan: null
                        }
                    }
                }
            }
        }

        Component {
            id: propertyTitleContentCom
            Label {
                anchors.centerIn: parent
                textFormat: Text.PlainText
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                font: DTK.fontManager.t8
                text: title
            }
        }

        ImageCell {
            source: "qrc:/assets/property/property-1.svg"
            DialogWindow {
                width: 280
                minimumHeight: 100
                maximumHeight: 1920
                header: DialogTitleBar {
                    enableInWindowBlendBlur: true
                    content: Loader {
                        sourceComponent: propertyTitleContentCom
                        property string title: "uos-icons-stration.png"
                    }
                }
                ColumnLayout {
                    id: contentHeight
                    anchors {
                        horizontalCenter: parent.horizontalCenter
                        margins: 10
                    }
                    Image {
                        source: "qrc:/assets/popup/nointeractive.svg"
                    }
                    PropertyItem {
                        title: "基本信息"
                        ColumnLayout {
                            spacing: 1
                            PropertyItemDelegate {
                                Layout.fillWidth: true
                                title: "文件名"
                                description: "uos-icons-demonstration.png"
                                iconName: "action_edit"
                                corners: RoundRectangle.TopCorner
                            }

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 1
                                PropertyItemDelegate {
                                    title: "大小"
                                    description: "4.23M"
                                    corners: RoundRectangle.BottomLeftCorner
                                }
                                PropertyItemDelegate {
                                    title: "尺寸"
                                    description: "1920X1080像素"
                                    Layout.fillWidth: true
                                }
                                PropertyItemDelegate {
                                    title: "图型"
                                    description: "图像"
                                    corners: RoundRectangle.BottomRightCorner
                                }
                            }
                        }
                        ColumnLayout {
                            spacing: 1
                            PropertyItemDelegate {
                                Layout.fillWidth: true
                                title: "添加日期"
                                description: "2014年4月23日 下午5：28"
                                corners: RoundRectangle.TopCorner
                            }

                            PropertyItemDelegate {
                                Layout.fillWidth: true
                                title: "图型"
                                description: "2016年8月27日 下午5：28"
                                corners: RoundRectangle.BottomCorner
                            }
                        }
                    }

                    PropertyItem {
                        title: "打开方式"
                        PropertyItemDelegate {
                            corners: RoundRectangle.AllCorner
                            title: "默认程序"
                            action: RowLayout {
                                spacing: 10
                                DciIcon {
                                    name: "entry_voice"
                                    width: 10
                                    height: 10
                                }
                                ComboBox {
                                    Layout.preferredWidth: 100
                                    font: DTK.fontManager.t8
                                    model: ["GIMP编辑器", "Code"]
                                }
                            }
                        }
                    }

                    PropertyItem {
                        title: "权限管理"
                        ColumnLayout {
                            spacing: 1
                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 1
                                PropertyItemDelegate {
                                    title: "所有者"
                                    corners: RoundRectangle.TopLeftCorner
                                    action: ComboBox {
                                        width: 70
                                        model: ["读写", "只读"]
                                    }
                                }
                                PropertyItemDelegate {
                                    title: "组"
                                    Layout.fillWidth: true
                                    action: ComboBox {
                                        width: 70
                                        model: ["读写", "只读"]
                                    }
                                }
                                PropertyItemDelegate {
                                    title: "其它"
                                    corners: RoundRectangle.TopRightCorner
                                    action: ComboBox {
                                        width: 70
                                        model: ["只读", "读写"]
                                    }
                                }
                            }

                            PropertyItemDelegate {
                                Layout.fillWidth: true
                                corners: RoundRectangle.BottomCorner
                                action: CheckBox {
                                    text: "允许以程序运行"
                                }
                            }
                        }
                    }

                    PropertyItem {
                        title: "共享管理"
                        ColumnLayout {
                            spacing: 1
                            PropertyItemDelegate {
                                Layout.fillWidth: true
                                action: CheckBox {
                                    text: "共享此文件"
                                }
                                corners: RoundRectangle.TopCorner
                            }

                            PropertyItemDelegate {
                                Layout.fillWidth: true
                                title: "共享名"
                                description: "uos-icons-demonstration.png"
                                iconName: "action_edit"
                            }

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 1
                                PropertyItemDelegate {
                                    title: "组"
                                    Layout.fillWidth: true
                                    corners: RoundRectangle.BottomLeftCorner
                                    action: ComboBox {
                                        width: 100
                                        model: ["读写", "只读"]
                                    }
                                }
                                PropertyItemDelegate {
                                    title: "匿名访问"
                                    corners: RoundRectangle.BottomRightCorner
                                    action: ComboBox {
                                        width: 100
                                        model: ["不允许", "允许"]
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        ImageCell {
            source: "qrc:/assets/property/property-2.svg"
            DialogWindow {
                width: 280
                minimumHeight: 100
                maximumHeight: 1920
                header: DialogTitleBar {
                    enableInWindowBlendBlur: true
                    content: Loader {
                        sourceComponent: propertyTitleContentCom
                        property string title: "设计稿"
                    }
                }
                ColumnLayout {
                    id: contentHeight2
                    anchors {
                        horizontalCenter: parent.horizontalCenter
                        margins: 10
                    }
                    Image {
                        source: "qrc:/assets/popup/nointeractive.svg"
                    }
                    PropertyItem {
                        title: "基本信息"
                        ColumnLayout {
                            spacing: 1
                            PropertyItemDelegate {
                                Layout.fillWidth: true
                                title: "文件名"
                                description: "设计稿"
                                iconName: "action_edit"
                                corners: RoundRectangle.TopCorner
                            }

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 1
                                PropertyItemDelegate {
                                    title: "大小"
                                    description: "186.5M"
                                    corners: RoundRectangle.BottomLeftCorner
                                }
                                PropertyItemDelegate {
                                    title: "文件数量"
                                    description: "172项"
                                    Layout.fillWidth: true
                                }
                                PropertyItemDelegate {
                                    title: "类型"
                                    description: "文件夹"
                                    corners: RoundRectangle.BottomRightCorner
                                }
                            }
                        }
                        ColumnLayout {
                            spacing: 1
                            PropertyItemDelegate {
                                Layout.fillWidth: true
                                title: "添加日期"
                                description: "2014年4月23日 下午5：28"
                                corners: RoundRectangle.TopCorner
                            }

                            PropertyItemDelegate {
                                Layout.fillWidth: true
                                title: "图型"
                                description: "2016年8月27日 下午5：28"
                                corners: RoundRectangle.BottomCorner
                            }
                        }
                    }

                    PropertyItem {
                        title: "权限管理"
                        ColumnLayout {
                            spacing: 1
                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 1
                                PropertyItemDelegate {
                                    title: "所有者"
                                    corners: RoundRectangle.TopLeftCorner
                                    action: ComboBox {
                                        width: 70
                                        model: ["读写", "只读"]
                                    }
                                }
                                PropertyItemDelegate {
                                    title: "组"
                                    Layout.fillWidth: true
                                    action: ComboBox {
                                        width: 70
                                        model: ["读写", "只读"]
                                    }
                                }
                                PropertyItemDelegate {
                                    title: "其它"
                                    corners: RoundRectangle.TopRightCorner
                                    action: ComboBox {
                                        width: 70
                                        model: ["只读", "读写"]
                                    }
                                }
                            }

                            PropertyItemDelegate {
                                Layout.fillWidth: true
                                corners: RoundRectangle.BottomCorner
                                action: CheckBox {
                                    text: "允许以程序运行"
                                }
                            }
                        }
                    }

                    PropertyItem {
                        title: "共享管理"
                        ColumnLayout {
                            spacing: 1
                            PropertyItemDelegate {
                                Layout.fillWidth: true
                                action: CheckBox {
                                    text: "共享此文件"
                                    checked: true
                                }
                                corners: RoundRectangle.TopCorner
                            }

                            PropertyItemDelegate {
                                Layout.fillWidth: true
                                title: "共享名"
                                description: "V23设计"
                                iconName: "action_edit"
                            }

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 1
                                PropertyItemDelegate {
                                    title: "组"
                                    Layout.fillWidth: true
                                    corners: RoundRectangle.BottomLeftCorner
                                    action: ComboBox {
                                        width: 100
                                        model: ["读写", "只读"]
                                    }
                                }
                                PropertyItemDelegate {
                                    title: "匿名访问"
                                    corners: RoundRectangle.BottomRightCorner
                                    action: ComboBox {
                                        width: 100
                                        model: ["不允许", "允许"]
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        Column {
            ImageCell {
                source: "qrc:/assets/property/property-3.svg"
                DialogWindow {
                    width: 280
                    minimumHeight: 100
                    maximumHeight: 1920
                    header: DialogTitleBar {
                        enableInWindowBlendBlur: true
                        content: Loader {
                            sourceComponent: propertyTitleContentCom
                            property string title: "RO-aj8924"
                        }
                    }
                    ColumnLayout {
                        id: contentHeight3
                        anchors {
                            horizontalCenter: parent.horizontalCenter
                            margins: 10
                        }
                        Image {
                            source: "qrc:/assets/popup/nointeractive.svg"
                        }
                        PropertyItem {
                            title: "基本信息"
                            ColumnLayout {
                                spacing: 1
                                PropertyItemDelegate {
                                    Layout.fillWidth: true
                                    title: "容量"
                                    description: "8G"
                                    corners: RoundRectangle.TopCorner
                                }
                                RowLayout {
                                    Layout.fillWidth: true
                                    spacing: 1
                                    PropertyItemDelegate {
                                        title: "容量"
                                        description: "186.5M"
                                        corners: RoundRectangle.BottomLeftCorner
                                    }
                                    PropertyItemDelegate {
                                        title: "文件数量"
                                        description: "1235项"
                                        Layout.fillWidth: true
                                    }
                                    PropertyItemDelegate {
                                        title: "类型"
                                        description: "只读光盘"
                                        corners: RoundRectangle.BottomRightCorner
                                    }
                                }
                            }
                            ColumnLayout {
                                spacing: 1
                                PropertyItemDelegate {
                                    Layout.fillWidth: true
                                    title: "添加日期"
                                    description: "2014年4月23日 下午5：28"
                                    corners: RoundRectangle.TopCorner
                                }

                                PropertyItemDelegate {
                                    Layout.fillWidth: true
                                    title: "图型"
                                    description: "2016年8月27日 下午5：28"
                                    corners: RoundRectangle.BottomCorner
                                }
                            }
                        }
                    }
                }
            }

            ImageCell {
                source: "qrc:/assets/property/property-4.svg"
                DialogWindow {
                    width: 280
                    minimumHeight: 100
                    maximumHeight: 1920
                    header: DialogTitleBar {
                        enableInWindowBlendBlur: true
                        content: Loader {
                            sourceComponent: propertyTitleContentCom
                            property string title: "IMG1234.jpg"
                        }
                    }
                    ColumnLayout {
                        id: contentHeight4
                        anchors {
                            horizontalCenter: parent.horizontalCenter
                            margins: 10
                        }
                        Image {
                            source: "qrc:/assets/popup/nointeractive.svg"
                        }
                        PropertyItem {
                            title: "基本信息"
                            ColumnLayout {
                                spacing: 1
                                PropertyItemDelegate {
                                    Layout.fillWidth: true
                                    title: "文件名"
                                    description: "IMG1234.jpg"
                                    iconName: "action_edit"
                                }
                                RowLayout {
                                    Layout.fillWidth: true
                                    spacing: 1
                                    PropertyItemDelegate {
                                        title: "光圈"
                                        description: "f2.8"
                                    }
                                    PropertyItemDelegate {
                                        title: "快门"
                                        description: "1/500s"
                                        Layout.fillWidth: true
                                    }
                                    PropertyItemDelegate {
                                        title: "ISO"
                                        description: "200"
                                    }
                                }
                                RowLayout {
                                    Layout.fillWidth: true
                                    spacing: 1
                                    PropertyItemDelegate {
                                        title: "大小"
                                        description: "16.8M"
                                        corners: RoundRectangle.BottomLeftCorner
                                    }
                                    PropertyItemDelegate {
                                        title: "分辨率"
                                        description: "8000X6000"
                                        Layout.fillWidth: true
                                    }
                                    PropertyItemDelegate {
                                        title: "自平衡"
                                        description: "自动"
                                        corners: RoundRectangle.BottomRightCorner
                                    }
                                }
                            }
                            ColumnLayout {
                                spacing: 1
                                PropertyItemDelegate {
                                    Layout.fillWidth: true
                                    title: "拍摄日期"
                                    description: "2014年4月23日 下午5：28"
                                    corners: RoundRectangle.TopCorner
                                }

                                PropertyItemDelegate {
                                    Layout.fillWidth: true
                                    title: "修改日期"
                                    description: "2016年8月27日 下午5：28"
                                    corners: RoundRectangle.BottomCorner
                                }
                            }
                        }
                        PropertyItem {
                            title: "详细信息"
                            ColumnLayout {
                                spacing: 1
                                RowLayout {
                                    Layout.fillWidth: true
                                    spacing: 1
                                    PropertyItemDelegate {
                                        title: "曝光"
                                        description: "自动"
                                        corners: RoundRectangle.TopLeftCorner
                                    }
                                    PropertyItemDelegate {
                                        title: "曝光程序"
                                        description: "光圈优先"
                                        Layout.fillWidth: true
                                        corners: RoundRectangle.TopRightCorner
                                    }
                                    PropertyItemDelegate {
                                        title: "曝光补偿"
                                        description: "-2"
                                    }
                                }
                                RowLayout {
                                    Layout.fillWidth: true
                                    spacing: 1
                                    PropertyItemDelegate {
                                        title: "焦距"
                                        description: "55mm"
                                        corners: RoundRectangle.BottomLeftCorner
                                    }
                                    PropertyItemDelegate {
                                        title: "闪光灯"
                                        description: "TTL"
                                        Layout.fillWidth: true
                                    }
                                    PropertyItemDelegate {
                                        title: "测光模式"
                                        description: "点测光"
                                        corners: RoundRectangle.BottomRightCorner
                                    }
                                }
                            }

                            PropertyItemDelegate {
                                title: "镜头型号"
                                description: "AF-S DX VR ZOOM      19-105mm f/2 5.5 6G"
                                corners: RoundRectangle.AllCorner
                            }
                        }
                    }
                }
            }
        }
    }
}
