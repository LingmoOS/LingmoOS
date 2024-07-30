/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import org.kde.lingmo.configuration 2.0
import QtQuick.Controls 2.3 as QQC2
import QtQuick.Layouts 1.1
import QtQml 2.15

import org.kde.newstuff 1.62 as NewStuff
import org.kde.lingmoui 2.20 as LingmoUI
import org.kde.kcmutils
import org.kde.lingmo.plasmoid 2.0
import org.kde.lingmo.configuration 2.0

Item {
    id: appearanceRoot
    signal configurationChanged

    property int formAlignment: wallpaperComboBox.LingmoUI.ScenePosition.x - appearanceRoot.LingmoUI.ScenePosition.x + LingmoUI.Units.smallSpacing
    property string currentWallpaper: ""
    property string containmentPlugin: ""
    property alias parentLayout: parentLayout

    function saveConfig() {
        if (main.currentItem.saveConfig) {
            main.currentItem.saveConfig()
        }
        configDialog.currentWallpaper = appearanceRoot.currentWallpaper;
        for (var key in configDialog.wallpaperConfiguration) {
            if (main.currentItem["cfg_"+key] !== undefined) {
                configDialog.wallpaperConfiguration[key] = main.currentItem["cfg_"+key]
            }
        }
        configDialog.applyWallpaper()
        configDialog.containmentPlugin = appearanceRoot.containmentPlugin
    }

    ColumnLayout {
        width: root.availableWidth
        height: Math.max(implicitHeight, root.availableHeight)
        spacing: 0 // unless it's 0 there will be an additional gap between two FormLayouts

        Component.onCompleted: {
            for (var i = 0; i < configDialog.containmentPluginsConfigModel.count; ++i) {
                var pluginName = configDialog.containmentPluginsConfigModel.data(configDialog.containmentPluginsConfigModel.index(i, 0), ConfigModel.PluginNameRole);
                if (configDialog.containmentPlugin === pluginName) {
                    pluginComboBox.currentIndex = i
                    pluginComboBox.activated(i);
                    break;
                }
            }

            for (var i = 0; i < configDialog.wallpaperConfigModel.count; ++i) {
                var pluginName = configDialog.wallpaperConfigModel.data(configDialog.wallpaperConfigModel.index(i, 0), ConfigModel.PluginNameRole);
                if (configDialog.currentWallpaper === pluginName) {
                    wallpaperComboBox.currentIndex = i
                    wallpaperComboBox.activated(i);
                    break;
                }
            }
        }

        LingmoUI.InlineMessage {
            visible: Plasmoid.immutable || animating
            text: i18nd("lingmo_shell_org.kde.lingmo.desktop", "Layout changes have been restricted by the system administrator")
            showCloseButton: true
            Layout.fillWidth: true
            Layout.leftMargin: LingmoUI.Units.smallSpacing
            Layout.rightMargin: LingmoUI.Units.smallSpacing
            Layout.bottomMargin: LingmoUI.Units.smallSpacing * 2 // we need this because ColumnLayout's spacing is 0
        }

        LingmoUI.FormLayout {
            id: parentLayout // needed for twinFormLayouts to work in wallpaper plugins
            twinFormLayouts: main.currentItem.formLayout || []
            Layout.fillWidth: true
            QQC2.ComboBox {
                id: pluginComboBox
                Layout.preferredWidth: Math.max(implicitWidth, wallpaperComboBox.implicitWidth)
                LingmoUI.FormData.label: i18nd("lingmo_shell_org.kde.lingmo.desktop", "Layout:")
                enabled: !Plasmoid.immutable
                model: configDialog.containmentPluginsConfigModel
                textRole: "name"
                onActivated: {
                    var model = configDialog.containmentPluginsConfigModel.get(currentIndex)
                    appearanceRoot.containmentPlugin = model.pluginName
                    appearanceRoot.configurationChanged()
                }
            }

            RowLayout {
                Layout.fillWidth: true
                enabled: main.currentItem.objectName !== "switchContainmentWarningItem"
                LingmoUI.FormData.label: i18nd("lingmo_shell_org.kde.lingmo.desktop", "Wallpaper type:")

                QQC2.ComboBox {
                    id: wallpaperComboBox
                    Layout.preferredWidth: Math.max(implicitWidth, pluginComboBox.implicitWidth)
                    model: configDialog.wallpaperConfigModel
                    textRole: "name"
                    onActivated: {
                        var idx = configDialog.wallpaperConfigModel.index(currentIndex, 0)
                        var pluginName = configDialog.wallpaperConfigModel.data(idx, ConfigModel.PluginNameRole)
                        if (appearanceRoot.currentWallpaper === pluginName) {
                            return;
                        }
                        appearanceRoot.currentWallpaper = pluginName
                        configDialog.currentWallpaper = pluginName
                        main.sourceFile = configDialog.wallpaperConfigModel.data(idx, ConfigModel.SourceRole)
                        appearanceRoot.configurationChanged()
                    }
                }
                NewStuff.Button {
                    configFile: "wallpaperplugin.knsrc"
                    text: i18nd("lingmo_shell_org.kde.lingmo.desktop", "Get New Plugins…")
                    visibleWhenDisabled: true // don't hide on disabled
                    Layout.preferredHeight: wallpaperComboBox.height
                }
            }
        }

        Item {
            id: emptyConfig
        }

        QQC2.StackView {
            id: main

            implicitHeight: main.empty ? 0 : currentItem.implicitHeight

            Layout.fillHeight: true;
            Layout.fillWidth: true;

            // Bug 360862: if wallpaper has no config, sourceFile will be ""
            // so we wouldn't load emptyConfig and break all over the place
            // hence set it to some random value initially
            property string sourceFile: "tbd"

            onSourceFileChanged: loadSourceFile()

            function loadSourceFile() {
                const wallpaperConfig = configDialog.wallpaperConfiguration
                // BUG 407619: wallpaperConfig can be null before calling `ContainmentItem::loadWallpaper()`
                if (wallpaperConfig && sourceFile) {
                    var props = {
                        "configDialog": configDialog
                    }

                    for (var key in wallpaperConfig) {
                        props["cfg_" + key] = wallpaperConfig[key]
                    }

                    var newItem = replace(Qt.resolvedUrl(sourceFile), props)

                    for (var key in wallpaperConfig) {
                        var changedSignal = newItem["cfg_" + key + "Changed"]
                        if (changedSignal) {
                            changedSignal.connect(appearanceRoot.configurationChanged)
                        }
                    }

                    const configurationChangedSignal = newItem.configurationChanged
                    if (configurationChangedSignal) {
                        configurationChangedSignal.connect(appearanceRoot.configurationChanged)
                    }
                } else {
                    replace(emptyConfig)
                }
            }
        }

        LingmoUI.Separator {
            Layout.fillWidth: true
        }
    }

    Component {
        id: switchContainmentWarning

        Item {
            objectName: "switchContainmentWarningItem"

            LingmoUI.PlaceholderMessage {
                id: message
                width: parent.width - LingmoUI.Units.largeSpacing * 8
                anchors.centerIn: parent

                icon.name: "documentinfo"
                text: i18nd("lingmo_shell_org.kde.lingmo.desktop", "Layout changes must be applied before other changes can be made")

                helpfulAction: QQC2.Action {
                    icon.name: "dialog-ok-apply"
                    text: i18nd("lingmo_shell_org.kde.lingmo.desktop", "Apply Now")
                    onTriggered: saveConfig()
                }
            }
        }
    }

    onContainmentPluginChanged: {
        if (configDialog.containmentPlugin !== appearanceRoot.containmentPlugin) {
            main.push(switchContainmentWarning);
            categoriesScroll.enabled = false;
        } else if (main.currentItem.objectName === "switchContainmentWarningItem") {
            main.pop();
            categoriesScroll.enabled = true;
        }
    }
}
