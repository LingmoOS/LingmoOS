// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.15
import org.lingmo.oceanui 1.0
import org.lingmo.dtk 1.0
import QtQml.Models

// 语言和区域
OceanUIObject {
    // 语言列表抬头
    OceanUIObject {
        id: languageListTiltle
        property bool isEditing: false
        name: "languageListTiltle"
        parentName: "langAndFormat"
        displayName: qsTr("Language")
        weight: 10
        pageType: OceanUIObject.Item
        page: RowLayout {
            Label {
                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                Layout.leftMargin: 20
                font: DTK.fontManager.t3
                text: oceanuiObj.displayName
            }

            Button {
                id: button
                checkable: true
                checked: languageListTiltle.isEditing
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                Layout.rightMargin: 10
                text: languageListTiltle.isEditing ? qsTr("done") : qsTr("edit")
                background: null
                enabled: oceanuiData.langState === 0
                textColor: Palette {
                    normal {
                        common: DTK.makeColor(Color.Highlight)
                        crystal: DTK.makeColor(Color.Highlight)
                    }
                }
                onCheckedChanged: {
                    languageListTiltle.isEditing = button.checked
                }
            }
        }

        // 语言列表项
        OceanUIObject {
            id: languageList
            name: "languageList"
            parentName: "langAndFormat"
            weight: 20
            pageType: OceanUIObject.Item
            page: OceanUIGroupView {
                Component.onCompleted: {
                    oceanuiData.ensureLangModel()
                }
            }

            onParentItemChanged: item => { if (item) item.topPadding = 10 }

            OceanUIRepeater {
                model: oceanuiData.langList
                delegate: OceanUIObject {
                    name: "languageItem" + index
                    parentName: "languageList"
                    displayName: modelData
                    weight: 20 + 10 * (index + 1)
                    backgroundType: OceanUIObject.Normal
                    pageType: OceanUIObject.Item
                    enabled: oceanuiData.langState === 0 // language set finished
                    page: ItemDelegate {
                        id: itemDelegate
                        property bool isCurrentLang: oceanuiData.currentLang === oceanuiObj.displayName
                        visible: oceanuiObj
                        hoverEnabled: true
                        implicitHeight: 50
                        icon.name: oceanuiObj.icon
                        checkable: false

                        Label {
                            id: langName
                            text: oceanuiObj.displayName
                            elide: Text.ElideRight
                            anchors {
                                left: itemDelegate.left
                                leftMargin: 20
                                top: itemDelegate.top
                                topMargin: (itemDelegate.height - langName.height) / 2
                            }
                        }

                        IconButton {
                            id: removeButton
                            visible: (itemDelegate.isCurrentLang && oceanuiObj.enabled) ||
                                     languageListTiltle.isEditing
                            icon.name: itemDelegate.isCurrentLang ? "sp_ok" : "list_delete"
                            icon.width: 24
                            icon.height: 24
                            implicitWidth: 36
                            implicitHeight: 36
                            anchors {
                                right: itemDelegate.right
                                rightMargin: 10
                                top: itemDelegate.top
                                topMargin: (itemDelegate.height - removeButton.height) / 2
                            }
                            background: null
                            onClicked: {
                                if (!languageListTiltle.isEditing
                                        || itemDelegate.isCurrentLang)
                                    return

                                oceanuiData.deleteLang(oceanuiObj.displayName)
                            }
                        }

                        Loader {
                            active: itemDelegate.isCurrentLang && !oceanuiObj.enabled
                            sourceComponent: BusyIndicator {
                                running: true
                                implicitWidth: 36
                                implicitHeight: 36
                            }
                            onLoaded: {
                                item.parent = itemDelegate
                                item.anchors.right = itemDelegate.right
                                item.anchors.rightMargin = 10
                                item.anchors.top = itemDelegate.top
                                item.anchors.topMargin = (itemDelegate.height - removeButton.height) / 2
                            }
                        }

                        background: OceanUIItemBackground {
                            separatorVisible: true
                        }

                        onClicked: {
                            if (languageListTiltle.isEditing)
                                return

                            oceanuiData.setCurrentLang(oceanuiObj.displayName)
                        }
                    }
                }
            }
        }
    }

    // 其他语言列表 +
    OceanUIObject {
        name: "otherLanguagesTitle"
        parentName: "langAndFormat"
        weight: 30
        pageType: OceanUIObject.Item
        page: OceanUIGroupView {}

        onParentItemChanged: item => { if (item) item.topPadding = 10 }

        OceanUIObject {
            name: "langItem0"
            parentName: "otherLanguagesTitle"
            displayName: qsTr("Other languages")
            weight: 10
            backgroundType: OceanUIObject.Normal
            pageType: OceanUIObject.Editor
            page: Button {
                implicitWidth: 80
                text: qsTr("add")
                LangsChooserDialog {
                    id: dialogLoader
                    viewModel: oceanuiData.langSearchModel()
                    onSelectedLang: function (lang) {
                        oceanuiData.addLang(lang)
                    }
                }

                onClicked: {
                    dialogLoader.active = true
                    languageListTiltle.isEditing = false
                }
            }
        }
    }

    // 区域格式抬头
    OceanUIObject {
        name: "langlistTiltle"
        parentName: "langAndFormat"
        displayName: qsTr("Region")
        weight: 40
        pageType: OceanUIObject.Item
        page: RowLayout {
            Label {
                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                Layout.leftMargin: 20
                text: oceanuiObj.displayName
                font: DTK.fontManager.t4
            }
        }

        onParentItemChanged: item => { if (item) item.topPadding = 10 }
    }

    // 地区
    OceanUIObject {
        name: "regions"
        parentName: "langAndFormat"
        weight: 45
        displayName: qsTr("Region")
        description: qsTr("Operating system and applications may provide you with local content based on your country and region")
        backgroundType: OceanUIObject.Normal
        pageType: OceanUIObject.Editor
        page: Item {
            implicitWidth: rowlayout.implicitWidth
            implicitHeight: rowlayout.implicitHeight
            RowLayout {
                id: rowlayout
                Label {
                    id: regionLabel
                    text: oceanuiData.region
                }
                IconLabel {
                    Layout.alignment: Qt.AlignRight | Qt.AlignHCenter
                    icon.name: "arrow_ordinary_right"
                }
            }

            MouseArea {
                id: mouseArea
                anchors.fill: parent

                RegionsChooserWindow {
                    id: regionWndow
                    viewModel: oceanuiData.regionSearchModel()
                    currentIndex: oceanuiData.currentRegionIndex
                    currentText: oceanuiData.region
                    onSelectedRegion: function (region) {
                        regionLabel.text = region
                        oceanuiData.setRegion(region)
                    }
                }

                onClicked: function (mouse) {
                    if (!regionWndow.isVisible()) {
                        regionWndow.show()
                    }

                    languageListTiltle.isEditing = false
                }
            }
        }

        onParentItemChanged: item => { if (item) item.topPadding = 10 }
    }

    // 区域格式
    OceanUIObject {
        name: "regionAndFormat"
        parentName: "langAndFormat"
        weight: 50
        displayName: qsTr("Region and format")
        description: qsTr("Operating system and applications may set date and time formats based on regional formats")
        backgroundType: OceanUIObject.Normal
        pageType: OceanUIObject.Editor
        page: Item {
            implicitWidth: layout.implicitWidth
            implicitHeight: layout.implicitHeight
            RowLayout {
                id: layout
                Label {
                    text: oceanuiData.currentLanguageAndRegion
                }
                IconLabel {
                    Layout.alignment: Qt.AlignRight | Qt.AlignHCenter
                    icon.name: "arrow_ordinary_right"
                }
            }

            MouseArea {
                anchors.fill: parent
                RegionFormatDialog {
                    id: regionDialog
                    currentIndex: oceanuiData.currentLanguageAndRegionIndex()
                    viewModel: oceanuiData.langRegionSearchModel()
                    onSelectedRegion: function (locale, lang) {
                        oceanuiData.setCurrentLocaleAndLangRegion(locale, lang)
                    }

                    Connections {
                        target: oceanuiData
                        function onCurrentLanguageAndRegionChanged(currentLanguageAndRegion) {
                            regionDialog.currentIndex = oceanuiData.currentLanguageAndRegionIndex()
                        }
                    }
                }

                onClicked: {
                    regionDialog.show()
                    languageListTiltle.isEditing = false
                }
            }
        }

        onParentItemChanged: item => { if (item) item.topPadding = 10 }
    }

    // 时间日期格式
    OceanUIObject {
        id: timeFormats
        name: "timeFormats"
        parentName: "langAndFormat"
        weight: 60
        pageType: OceanUIObject.Item
        page: OceanUIGroupView {}

        onParentItemChanged: item => { if (item) item.topPadding = 10 }

        OceanUIRepeater {
            id: timeFormatsRepeater
            model: oceanuiData.timeDateModel()
            delegate: OceanUIObject {
                name: model.name
                parentName: "timeFormats"
                displayName: model.name
                weight: 10 * (index + 1)
                backgroundType: OceanUIObject.Normal
                pageType: OceanUIObject.Editor
                page: ComboLabel {
                    comboModel: model.values
                    comboCurrentIndex: model.current
                    onComboBoxActivated: function (idx) {
                        oceanuiData.setCurrentFormat(model.indexBegin + index, idx)
                    }
                }
            }
        }
    }

    // 货币符号格式
    OceanUIObject {
        id: currencyFormats
        name: "currencyFormats"
        parentName: "langAndFormat"
        weight: 70
        pageType: OceanUIObject.Item
        page: OceanUIGroupView {}

        onParentItemChanged: item => { if (item) item.topPadding = 10 }

        OceanUIRepeater {
            id: currencyRepeater
            model: oceanuiData.currencyModel()
            delegate: OceanUIObject {
                name: model.name
                parentName: "currencyFormats"
                displayName: model.name
                weight: 10 * (index + 1)
                backgroundType: OceanUIObject.Normal
                pageType: OceanUIObject.Editor
                page: ComboLabel {
                    comboModel: model.values
                    comboCurrentIndex: model.current
                    onComboBoxActivated: function (idx) {
                        oceanuiData.setCurrentFormat(model.indexBegin + index, idx)
                    }
                }
            }
        }
    }

    // 数字格式符号
    OceanUIObject {
        id: numberFormats
        name: "numberFormats"
        parentName: "langAndFormat"
        weight: 80
        pageType: OceanUIObject.Item
        page: OceanUIGroupView {}

        onParentItemChanged: item => { if (item) item.topPadding = 10 }

        OceanUIRepeater {
            id: numRepeater
            model: oceanuiData.decimalModel()
            delegate: OceanUIObject {
                name: model.name
                parentName: "numberFormats"
                displayName: model.name
                weight: 10 * (index + 1)
                backgroundType: OceanUIObject.Normal
                pageType: OceanUIObject.Editor
                page: ComboLabel {
                    comboModel: model.values
                    comboCurrentIndex: model.current
                    onComboBoxActivated: function (idx) {
                        oceanuiData.setCurrentFormat(model.indexBegin + index, idx)
                    }
                }
            }
        }
    }
}
