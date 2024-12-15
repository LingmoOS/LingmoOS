// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.15

import org.lingmo.oceanui 1.0
import org.lingmo.dtk 1.0 as D

OceanUIObject {
    OceanUIObject {
        name: "themeGroup"
        parentName: "personalization"
        weight: 10
        pageType: OceanUIObject.Item
        page: OceanUIGroupView {}

        OceanUIObject {
            name: "themeTitle"
            parentName: "personalization/themeGroup"
            displayName: qsTr("Theme")
            weight: 1
            pageType: OceanUIObject.Item
            page: ColumnLayout {
                Layout.fillHeight: true
                Layout.margins: 10
                RowLayout {
                    Layout.fillWidth: true
                    Label {
                        Layout.topMargin: 10
                        font: D.DTK.fontManager.t5
                        text: oceanuiObj.displayName
                        Layout.leftMargin: 10
                    }

                    Item {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                    }

                    D.IconButton {
                        flat: true
                        enabled: themeSelectView.currentIndex !== 0
                        icon.name: "arrow_ordinary_left"
                        onClicked: {
                            themeSelectView.decrementCurrentIndex()
                        }
                    }
                    D.IconButton {
                        flat: true
                        enabled: themeSelectView.currentIndex !== themeSelectView.count - 1
                        icon.name: "arrow_ordinary_right"
                        onClicked: {
                            themeSelectView.incrementCurrentIndex()
                        }
                    }
                }
                ThemeSelectView {
                    id: themeSelectView
                    Layout.fillWidth: true
                    Layout.preferredHeight: 320
                }
            }
        }

        OceanUIObject {
            name: "appearance"
            parentName: "personalization/themeGroup"
            displayName: qsTr("Appearance")
            description: qsTr("Select light, dark, or automatic theme appearance")
            weight: 2
            pageType: OceanUIObject.Editor
            icon: "appearance"
            page: D.ComboBox {
                flat: true
                textRole: "text"
                model: oceanuiData.appearanceSwitchModel
                currentIndex: {
                    for (var i = 0; i < model.length; ++i) {
                        if (model[i].value === oceanuiData.currentAppearance) {
                            return i;
                        }
                    }
                    return 0
                }

                enabled: model.length > 1
                onCurrentIndexChanged: {
                    oceanuiData.worker.setAppearanceTheme(model[currentIndex].value)
                }
            }
        }
    }

    OceanUIObject {
        name: "windowEffect"
        parentName: "personalization"
        displayName: qsTr("Window effect")
        description: qsTr("Interface and effects、rounded corner、compact display")
        icon: "window_effect"
        weight: 200
        WindowEffectPage {}
    }
    OceanUIObject {
        name: "wallpaper"
        parentName: "personalization"
        displayName: qsTr("Wallpaper and screensaver")
        description: qsTr("Personalize your wallpaper and screensaver")
        icon: "wallpaper"
        weight: 300
        WallpaperPage {}
    }
    OceanUIObject {
        name: "colorAndIcons"
        parentName: "personalization"
        displayName: qsTr("Colors and icons")
        description: qsTr("Adjust accent color and theme icons")
        icon: "icon_cursor"
        weight: 400
        ColorAndIcons {}
    }
    OceanUIObject {
        name: "font"
        parentName: "personalization"
        displayName: qsTr("Font and font size")
        description: qsTr("Change system font and size")
        icon: "font_size"
        weight: 500
        FontSizePage {}
    }
}
