/*
 *   SPDX-FileCopyrightText: 2015 Marco Martin <mart@kde.org>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick 2.5
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.1

import org.kde.plasma.core as PlasmaCore
import org.kde.kirigami 2.20 as Kirigami
import org.kde.kitemmodels as KItemModels

Kirigami.AbstractApplicationWindow {
    id: root
    width: Kirigami.Units.gridUnit * 50
    height: Kirigami.Units.gridUnit * 35
    visible: true
    property int iconSize: iconSizeSlider.value
    property alias showMargins: showMarginsCheckBox.checked

    Shortcut {
        sequence: StandardKey.Quit
        onActivated: Qt.quit()
    }

    header: ToolBar {
        RowLayout {
            anchors.fill: parent
            ToolButton {
                ToolTip.text: i18n("New Theme…")
                icon.name: "document-new"
                onClicked: {
                    if (!root.metadataEditor) {
                        root.metadataEditor = metadataEditorComponent.createObject(root);
                    }
                    root.metadataEditor.newTheme = true;
                    root.metadataEditor.name = "";
                    root.metadataEditor.author = "";
                    root.metadataEditor.email = "";
                    root.metadataEditor.license = "LGPL 2.1+";
                    root.metadataEditor.website = "";
                    root.metadataEditor.open();
                }
            }
            Label {
                text: i18n("Theme:")
            }
            ComboBox {
                id: themeSelector
                //FIXME: why crashes?
                //model: 3//themeModel.themeList
                textRole: "display"
                onCurrentIndexChanged: {
                    themeModel.theme = themeModel.themeList.get(currentIndex).packageNameRole;
                }
            }
            ToolButton {
                ToolTip.text: i18n("Open Folder")
                icon.name: "document-open-folder"
                onClicked: Qt.openUrlExternally(themeModel.themeFolder);
            }
            ToolButton {
                ToolTip.text: i18n("Edit Metadata…")
                icon.name: "configure"
                enabled: view.currentItem?.modelData.isWritable ?? false
                onClicked: {
                    if (!root.metadataEditor) {
                        root.metadataEditor = metadataEditorComponent.createObject(root);
                    }
                    root.metadataEditor.newTheme = false;
                    root.metadataEditor.name = themeModel.theme;
                    root.metadataEditor.author = themeModel.author;
                    root.metadataEditor.email = themeModel.email;
                    root.metadataEditor.license = themeModel.license;
                    root.metadataEditor.website = themeModel.website;
                    root.metadataEditor.open();
                }
            }
            ToolButton {
                ToolTip.text: i18n("Edit Colors…")
                icon.name: "color"
                enabled: view.currentItem?.modelData.isWritable ?? false
                onClicked: {
                    if (!root.colorEditor) {
                        root.colorEditor = colorEditorComponent.createObject(root);
                    }

                    root.colorEditor.open();
                }
            }
            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
            }
            ToolButton {
                ToolTip.text: i18n("Help")
                icon.name: "help-contents"
                onClicked: Qt.openUrlExternally("https://techbase.kde.org/Development/Tutorials/Plasma5/ThemeDetails");
            }
            TextField {
                placeholderText: i18n("Search…")
                onTextChanged: searchModel.filterRegularExpression = RegExp(".*" + text + ".*")
            }
        }
    }

    property QtObject metadataEditor
    Component {
        id: metadataEditorComponent
        MetadataEditor {}
    }
    property QtObject colorEditor
    Component {
        id: colorEditorComponent
        ColorEditor {}
    }

    Timer {
        running: true
        interval: 200
        onTriggered: {
            themeSelector.model = themeModel.themeList
            for (var i = 0; i < themeModel.themeList.count; ++i) {
                if (commandlineTheme == themeModel.themeList.get(i).packageNameRole) {
                    themeSelector.currentIndex = i;
                    break;
                }
            }
            //NOTE:assigning this in a second moment solves a crash in some versions of Qt 5.8
            searchModel.sourceModel= themeModel
        }
    }
    SystemPalette {
        id: palette
    }

    Rectangle {
        anchors.fill: scrollView
        Kirigami.Theme.colorSet: Kirigami.Theme.View
        color: Kirigami.Theme.backgroundColor
    }
    ScrollView {
        id: scrollView
        anchors {
            top: parent.top
            bottom: parent.bottom
            left: parent.left
            right: sidebar.left
        }
        GridView {
            id: view
            anchors.fill: parent
            model: KItemModels.KSortFilterProxyModel {
                id: searchModel
                filterRoleName: "imagePath"
            }
            cellWidth: root.iconSize
            cellHeight: cellWidth
            highlightMoveDuration: 0

            highlight: Rectangle {
                radius: 3
                color: palette.highlight
            }
            delegate: Item {
                width: view.cellWidth
                height: view.cellHeight
                property QtObject modelData: model
                MouseArea {
                    z: 2
                    anchors.fill: parent
                    onClicked: {
                        view.currentIndex = index;
                    }
                }
                Loader {
                    z: -1
                    anchors.fill: parent
                    source: Qt.resolvedUrl("delegates/" + model.delegate + ".qml")
                }
                Rectangle {
                    anchors {
                        right: parent.right
                        bottom: parent.bottom
                        margins: Kirigami.Units.gridUnit
                    }
                    width: Kirigami.Units.gridUnit
                    height: Kirigami.Units.gridUnit
                    radius: Kirigami.Units.gridUnit
                    opacity: 0.5
                    color: model.usesFallback ? "red" : "green"
                }
            }
        }
    }
    Item {
        id: sidebar
        anchors {
            top: parent.top
            bottom: parent.bottom
            right: parent.right
        }
        width: root.width / 3
        Rectangle {
            width: 1
            anchors {
                top: parent.top
                bottom: parent.bottom
            }
            color: palette.highlight
        }
        ColumnLayout {
            anchors {
                fill: parent
                margins: Kirigami.Units.gridUnit
            }
            Label {
                Layout.fillWidth: true
                visible: !view.currentItem?.modelData.isWritable ?? false
                text: i18n("This is a readonly, system wide installed theme")
                wrapMode: Text.WordWrap
            }
            Label {
                Layout.fillWidth: true
                text: i18n("Preview:")
            }
            Loader {
                id: extendedLoader
                property QtObject model: view.currentItem?.modelData ?? null
                Layout.fillWidth: true
                Layout.minimumHeight: width
                source: model ? Qt.resolvedUrl("delegates/" + model.delegate + ".qml") : ""
            }
            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
            }
            Label {
                Layout.fillWidth: true
                text: i18n("Image path: %1", view.currentItem?.modelData.imagePath ?? i18n("None"))
                wrapMode: Text.WordWrap
            }
            Label {
                Layout.fillWidth: true
                text: i18n("Description: %1", view.currentItem?.modelData.description ?? "")
                wrapMode: Text.WordWrap
            }
            Label {
                Layout.fillWidth: true
                text: view.currentItem && view.currentItem.modelData.usesFallback ? i18n("Missing from this theme") : i18n("Present in this theme")
                wrapMode: Text.WordWrap
            }
            CheckBox {
                id: showMarginsCheckBox
                text: i18n("Show Margins")
            }
            Button {
                text: view.currentItem && view.currentItem.modelData.usesFallback ? i18n("Create with Editor…") : i18n("Open In Editor…")
                enabled: view.currentItem?.modelData.isWritable ?? false
                Layout.alignment: Qt.AlignHCenter
                onClicked: {
                    print(view.currentItem.modelData.svgAbsolutePath)
                    themeModel.editElement(view.currentItem.modelData.imagePath)
                    //Qt.openUrlExternally(view.currentItem.modelData.svgAbsolutePath)
                }
            }
            Slider {
                id: iconSizeSlider
                Layout.fillWidth: true
                value: Kirigami.Units.gridUnit * 12
                from: Kirigami.Units.gridUnit * 5
                to: Kirigami.Units.gridUnit * 20
            }
        }
    }
}
