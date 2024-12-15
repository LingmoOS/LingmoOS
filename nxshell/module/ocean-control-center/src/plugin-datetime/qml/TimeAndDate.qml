// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.15
import QtQml.Models 2.11
import org.lingmo.oceanui 1.0
import org.lingmo.dtk 1.0
import org.lingmo.dtk.private 1.0 as P

// 时间和日期
OceanUIObject {
    OceanUIObject {
        id: dateTimeContent
        name: "dateTimeContent"
        parentName: "timeAndLang"
        weight: 10
        pageType: OceanUIObject.Item
        page: ColumnLayout {

            FontLoader {
                id: webFont
                source: "qrc:/builtin-font/resource/Outfit-Light.ttf"
            }

            Label {
                id: timeLabel
                height: contentHeight
                Layout.leftMargin: 10
                font {
                    pointSize: 40
                    family: webFont.font.family
                 }
                text: oceanuiData.currentTime
            }
            Label {
                id: dateLabel
                height: contentHeight
                Layout.leftMargin: 10
                font {
                    pointSize: 14
                    family: webFont.font.family
                }
                text: oceanuiData.currentDate
            }
        }
    }

    // 时间同步
    OceanUIObject {
        name: "dateTimeGroup"
        parentName: "timeAndLang"
        weight: 12
        pageType: OceanUIObject.Item
        page: OceanUIGroupView {}

        onParentItemChanged: item => { if (item) item.topPadding = 10 }

        OceanUIObject {
            id: ntpSettings
            property bool ntpOn: oceanuiData.ntpEnabled
            name: "ntpSettings"
            parentName: "dateTimeGroup"
            displayName: qsTr("Auto sync time")
            weight: 10
            backgroundType: OceanUIObject.Normal
            pageType: OceanUIObject.Editor
            page: Switch {
                checked: ntpSettings.ntpOn
                onCheckedChanged: {
                    oceanuiData.ntpEnabled = checked
                }
            }
        }

        OceanUIObject {
            id: dateAndTimeSettings
            name: "dateAndTimeSettings"
            parentName: "dateTimeGroup"
            displayName: oceanuiData.ntpEnabled ? qsTr("Ntp server") : qsTr("System date and time")
            weight: 12
            backgroundType: OceanUIObject.Normal
            pageType: OceanUIObject.Editor
            property bool showCustom: false
            property string customAddr
            page: Item {
                implicitHeight: 36
                implicitWidth: oceanuiData.ntpEnabled ? 280 : 80

                ComboBox {
                    id: comboBox
                    property var serverList: oceanuiData.ntpServerList
                    flat: true
                    visible: oceanuiData.ntpEnabled
                    anchors.fill: parent
                    hoverEnabled: true
                    model: serverList
                    // 不设置默认的话可能无法滚动（不显示上下箭头按钮）。。。
                    maxVisibleItems: serverList.length - 1
                    currentIndex:  {
                        let index = serverList.indexOf(oceanuiData.ntpServerAddress)
                        dateAndTimeSettings.showCustom = (index < 0)
                        if (index < 0)
                            dateAndTimeSettings.customAddr = oceanuiData.ntpServerAddress
                        return index < 0 ? serverList.length - 1 : index
                    }
                    onActivated: function (index) {
                        dateAndTimeSettings.showCustom = (serverList[index] === qsTr("Customize"))
                        if (dateAndTimeSettings.showCustom) {
                            if (dateAndTimeSettings.customAddr.length > 0)
                                oceanuiData.ntpServerAddress = dateAndTimeSettings.customAddr
                            return
                        }

                        oceanuiData.ntpServerAddress = serverList[index]
                    }

                    Component.onCompleted: {
                        let text = qsTr("Customize")
                        if (!comboBox.serverList.includes(text))
                            comboBox.serverList.push(text)

                        if (comboBox.currentIndex < 0) {
                            comboBox.currentIndex = comboBox.serverList.length - 1
                            dateAndTimeSettings.showCustom = true
                            dateAndTimeSettings.customAddr = oceanuiData.ntpServerAddress
                        }
                    }
                }

                Button {
                    id: settingsButton
                    visible: !oceanuiData.ntpEnabled
                    anchors.fill: parent
                    property bool needShowDialog: false
                    text: qsTr("Settings")

                    Loader {
                        id: loader
                        active: settingsButton.needShowDialog
                        sourceComponent: DateTimeSettingDialog {
                            onClosing: {
                                settingsButton.needShowDialog = false
                            }
                        }
                        onLoaded: {
                            item.show()
                        }
                    }

                    onClicked: {
                        settingsButton.needShowDialog = true
                    }
                }
            }
            onDeactive: {
                if (dateAndTimeSettings.showCustom &&
                        dateAndTimeSettings.customAddr.length === 0) {
                    dateAndTimeSettings.showCustom = false
                }
            }
        }
        OceanUIObject {
            id: customNTPServer
            name: "customNTPServer"
            parentName: "dateTimeGroup"
            displayName: qsTr("Server address")
            visible: oceanuiData.ntpEnabled && dateAndTimeSettings.showCustom
            weight: 13
            backgroundType: OceanUIObject.Normal
            pageType: OceanUIObject.Editor
            page: Item {
                id: item
                implicitHeight: 50
                implicitWidth: 300
                LineEdit {
                    id: addr
                    implicitWidth: 200
                    text: dateAndTimeSettings.customAddr
                    placeholderText: qsTr("Required")
                    alertText: qsTr("The ntp server address cannot be empty")
                    anchors{
                        rightMargin: 10
                        right: editBtn.left
                        verticalCenter: parent.verticalCenter
                    }
                    onReadOnlyChanged: {
                        addr.background.visible = !addr.readOnly
                        addr.clearButton.visible = !addr.readOnly && text.length > 0
                        addr.focus = !addr.readOnly
                    }
                    onTextChanged: {
                        if (addr.showAlert && addr.text.trim().length > 0) {
                            addr.showAlert = false
                        }
                    }
                    Component.onCompleted: {
                        Qt.callLater( function(){ addr.forceActiveFocus() } )
                        addr.readOnly = text.length > 0
                    }
                }
                IconButton {
                    id: editBtn
                    enabled: addr.text.length > 0
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    icon {
                        name: addr.readOnly ? "edit" : "ok"
                        width: 16
                        height: 16
                    }
                    onClicked: {
                        if (addr.text.trim().length === 0) {
                            addr.showAlert = true
                            return
                        }

                        addr.showAlert = false

                        if (!addr.readOnly) {
                            oceanuiData.ntpServerAddress = addr.text
                        }

                        addr.readOnly = !addr.readOnly
                    }
                }
            }
        }
        OceanUIObject {
            visible: false // 暂时隐藏，会导致逻辑很复杂
            name: "12/24h"
            parentName: "dateTimeGroup"
            displayName: qsTr("Use 24-hour format")
            weight: 14
            backgroundType: OceanUIObject.Normal
            pageType: OceanUIObject.Editor
            page: Switch {
                checked: oceanuiData.use24HourFormat
                onCheckedChanged: {
                    oceanuiData.use24HourFormat = checked
                }
            }
        }
    }

    // 系统时区
    OceanUIObject {
        id: timezoneGroup
        name: "timezoneGroup"
        parentName: "timeAndLang"
        weight: 20
        pageType: OceanUIObject.Item
        page: OceanUIGroupView {
            id: view
        }

        OceanUIRepeater {
            id: userTimezoneRepeater
            model: oceanuiData.userTimezoneModel()
            delegate: ItemZoneComp {
                name: "userTimezoneItem" + index
                parentName: "timezoneGroup"
                displayName: model.display
                description: model.description
                weight: 20 + 10 * (index + 1)
                shift: model.shift
                zoneId: model.zoneId
            }
        }

        onParentItemChanged: item => { if (item) item.topPadding = 10 }
    }

    OceanUIObject {
        id: systemTimezone
        name: "systemTimezone"
        parentName: "timezoneGroup"
        displayName: qsTr("system time zone")
        weight: 12
        backgroundType: OceanUIObject.Normal
        pageType: OceanUIObject.Editor
        page: ComboBox {
            id: combo
            flat: true
            implicitWidth: 280
            model: oceanuiData.zoneSearchModel()
            textRole: "display"
            displayText: oceanuiData.timeZoneDispalyName
            hoverEnabled: true
            currentIndex: oceanuiData.currentTimeZoneIndex

            popup: SearchableListViewPopup {
                id: searchView
                implicitWidth: combo.width
                delegateModel: combo.delegateModel
                maxVisibleItems: combo.maxVisibleItems
                highlightedIndex: combo.highlightedIndex
                onSearchTextChanged: {
                    let delegateModel = oceanuiData.zoneSearchModel()
                    delegateModel.setFilterWildcard(searchView.searchText);
                }
            }

            onActivated: function (index) {
                let zoneId = currentValue["zoneId"]
                oceanuiData.setSystemTimeZone(zoneId)
            }
        }
    }

    OceanUIObject {
        name: "timezoneList"
        parentName: "timezoneGroup"
        displayName: qsTr("Timezone list")
        weight: 12
        backgroundType: OceanUIObject.Normal
        pageType: OceanUIObject.Editor
        page: RowLayout {
            spacing: 10
            Button {
                id: addButton
                property bool needShowDialog: false
                icon.name: "add"
                implicitHeight: 32
                implicitWidth: 32
                Loader {
                    id: customLoader
                    active: false
                    sourceComponent: SearchableListViewPopup {
                        id: popup
                        implicitWidth: 280
                        delegateModel: DelegateModel {
                            model: oceanuiData.zoneSearchModel()
                            delegate: MenuItem {
                                useIndicatorPadding: true
                                width: popup.width
                                text: model.display
                                highlighted: hovered
                                hoverEnabled: true
                                checkable: true
                                autoExclusive: true
                                onHoveredChanged: {
                                    if (hovered)
                                        popup.highlightedIndex = index
                                }
                                onCheckedChanged: {
                                    let zoneId = model.zoneId
                                    oceanuiData.addUserTimeZoneById(zoneId)
                                    popup.close()
                                }
                            }
                        }
                        maxVisibleItems: 16
                        onSearchTextChanged: {
                            let delegateModel = oceanuiData.zoneSearchModel()
                            delegateModel.setFilterWildcard(popup.searchText);
                        }
                        onClosed: {
                            customLoader.active = false
                        }
                    }
                    onLoaded: {
                        item.open()
                        item.x = addButton.implicitWidth - item.implicitWidth + 10
                    }
                }

                onClicked: {
                    customLoader.active = true
                }
            }
        }
    }

    component ItemZoneComp: OceanUIObject {
        property int shift: 8
        property string zoneId
        backgroundType: OceanUIObject.Normal
        pageType: OceanUIObject.Item
        page: ItemDelegate {
            id: itemDelegate
            visible: oceanuiObj
            hoverEnabled: true
            implicitHeight: 50
            icon.name: oceanuiObj.icon
            checkable: false
            RowLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                ColumnLayout {
                    Layout.topMargin: 8
                    Layout.leftMargin: 60
                    Layout.maximumWidth: 160

                    Label {
                        id: display
                        Layout.maximumWidth: 160
                        text: oceanuiObj.displayName
                        elide: Text.ElideRight
                    }
                    Label {
                        id: description
                        Layout.maximumWidth: 160
                        visible: text !== ""
                        font: DTK.fontManager.t10
                        text: oceanuiObj.description
                        opacity: 0.5
                        elide: Text.ElideRight
                    }
                }
            }

            TimezoneClock {
                id: clock
                width: 36
                height: 36
                shift: oceanuiObj.shift
                anchors {
                    left: itemDelegate.left
                    leftMargin: 10
                    top: itemDelegate.top
                    topMargin: (itemDelegate.height - clock.height) / 2
                }
            }

            IconButton {
                id: removeButton
                visible: itemDelegate.hovered
                icon.name: "user-trash-symbolic"
                icon.width: 24
                icon.height: 24
                implicitWidth: 36
                implicitHeight: 36
                anchors {
                    right: itemDelegate.right
                    rightMargin: 10
                    verticalCenter: itemDelegate.verticalCenter
                }
                background: null
                onClicked: {
                    console.log("need remove timezone", oceanuiObj.displayName)
                    oceanuiData.removeUserTimeZoneById(oceanuiObj.zoneId)
                }
            }
            background: OceanUIItemBackground {
                separatorVisible: true
            }
        }
    }
}
