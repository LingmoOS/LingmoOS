/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQml.Models 2.12
import QtQuick.Controls 2.5
import org.lingmo.menu.core 1.0
import AppControls2 1.0 as AppControls2
import org.lingmo.quick.platform 1.0 as Platform
import org.lingmo.quick.items 1.0 as LingmoUIItems

RowLayout {
    id: root
    clip: true
    signal openFolderSignal(string folderId, string folderName, int x, int y)
    signal contentShowFinished()
    property bool isContentShow: true
    property int animationDuration: 300

    function viewFocusEnable() {
        contentViewLoader.focus = true;
    }
    function clearViewFocus() {
        contentViewLoader.focus = false;
        if (contentViewLoader.item.currentIndex) {
            contentViewLoader.item.currentIndex = 0;
        }
    }

    Component.onCompleted: mainWindow.visibleChanged.connect(clearViewFocus)
    Component.onDestruction: mainWindow.visibleChanged.disconnect(clearViewFocus)

    state: isContentShow ? "contentShow" : "contentHidden"
    states: [
        State {
            name: "contentHidden"
            PropertyChanges { target: root; opacity: 0; scale: 0.95 }
        },
        State {
            name: "contentShow"
            PropertyChanges { target: root; opacity: 1; scale: 1 }
        }
    ]

    transitions: [
        Transition {
            to:"contentHidden"
            SequentialAnimation {
                PropertyAnimation { properties: "opacity, scale"; duration: animationDuration; easing.type: Easing.InOutCubic }
                ScriptAction { script: root.visible = false }
            }
        },
        Transition {
            to: "contentShow"
                PropertyAnimation { properties: "opacity, scale"; duration: animationDuration; easing.type: Easing.InOutCubic }
        }
    ]

    Item {
        Layout.preferredWidth: 145
        Layout.fillHeight: true
        Layout.leftMargin: 15

        ListView {
            id: labelListView
            signal labelClicked(string labelId)
            property int maxLabelWidth: count < 20 ? width : 30

            anchors.centerIn: parent
            width: parent.width
            height: contentHeight > parent.height ? parent.height : contentHeight
            interactive: contentHeight > parent.height

            highlightMoveDuration: animationDuration
            highlight: LingmoUIItems.StyleBackground {
                width: labelListView.maxLabelWidth; height: 30
                radius: Platform.Theme.minRadius
                useStyleTransparency: false
                paletteRole: Platform.Theme.Light
                border.width: 1
                alpha: 0.18; borderAlpha: 0.7
                borderColor: Platform.Theme.HighlightedText
            }
            onCountChanged: currentIndex = 0

            model: DelegateModel {
                groups: DelegateModelGroup {
                    name: "disabledItem"
                    includeByDefault: false
                }

                items.onChanged: {
                    for (let i = 0; i < items.count;) {
                        let item = items.get(i);
                        if (item.model.isDisable) {
                            items.setGroups(i, 1, "disabledItem");
                            continue;
                        }
                        ++i;
                    }
                }

                model: modelManager.getLabelModel()
                delegate: MouseArea {
                    width: labelListView.maxLabelWidth
                    height: 30
                    hoverEnabled: true
                    LingmoUIItems.StyleText {
                        anchors.fill: parent
                        paletteRole: Platform.Theme.HighlightedText
                        elide: Text.ElideRight
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        text: model.displayName
                    }
                    onClicked: {
                        labelListView.currentIndex = DelegateModel.itemsIndex;
                        labelListView.labelClicked(model.id)
                    }
                }
            }
        }
    }

    Item {
        id: contentViewBase
        Layout.fillWidth: true
        Layout.fillHeight: true
        property int maxColumns: 8
        property int columns: {
            let c = Math.floor(width / cellWidth);
            return c > maxColumns ? maxColumns : c;
        }
        property int cellWidth: 220
        // property int cellWidth: Math.min(Math.floor(width / columns), 220)
        property int cellHeight: cellWidth
        property int headerHeight: 30

        Component.onCompleted: {
            changeView(modelManager.getLabelGroupModel().containLabel);
            modelManager.getLabelGroupModel().containLabelChanged.connect(changeView);
        }

        Component.onDestruction: {
            modelManager.getLabelGroupModel().containLabelChanged.disconnect(changeView);
            contentViewLoader.sourceComponent = undefined;
        }

        function changeView(toLabelView) {
            contentViewLoader.sourceComponent = toLabelView ? labelViewComponent : appViewComponent;
        }

        Loader {
            id: contentViewLoader
            height: parent.height
            width: contentViewBase.cellWidth * contentViewBase.columns + 2
            anchors.horizontalCenter: parent.horizontalCenter
        }

        // view id: 1
        Component {
            id: appViewComponent
            GridView {
                id: appGridView
                property string selectId: ""
                ScrollBar.vertical: fullScreenScrollBar
                cellWidth: contentViewBase.cellWidth
                cellHeight: contentViewBase.cellHeight
                boundsBehavior: Flickable.StopAtBounds
                model: modelManager.getAppModel()
                cacheBuffer: cellHeight * appGridView.count / 6
                focus: true
                keyNavigationEnabled: true

                delegate: Loader {
                    width: GridView.view.cellWidth
                    height: width
                    focus: true
                    property int index: model.index
                    property int type: model.type
                    property string id: model.id
                    property string name: model.name
                    property string icon: model.icon
                    sourceComponent: {
                        if (type === DataType.Normal) {
                            return appComponent;
                        }
                        if (type === DataType.Folder) {
                            return normalFolderComponent;
                        }
                    }
                }

                Component {
                    id: appComponent
                    DropArea {
                        id: dropArea
                        property int originalX
                        property int originalY
                        onEntered: {
                            if (appGridView.selectId !== id) {
                                imageBase.visible = true;
                            }
                        }
                        onExited: {
                            imageBase.visible = false;
                        }
                        onDropped: {
                            if (appGridView.selectId !== id) {
                                appGridView.model.addAppsToFolder(appGridView.selectId, id, "");
                            }
                        }

                        // ********按键导航********
                        focus: true
                        states: State {
                            when: dropArea.activeFocus
                            PropertyChanges {
                                target: controlBase
                                borderColor: Platform.Theme.Highlight
                                border.width: 2
                            }
                        }
                        Keys.onPressed: {
                            if (event.key === Qt.Key_Enter || event.key === Qt.Key_Return) {
                                appGridView.model.appClicked(index);
                            }
                        }

                        Item {
                            id: appItem
                            property bool isSelect: false
                            property bool isEnterd: false
                            width: contentViewBase.cellWidth
                            height: contentViewBase.cellHeight
                            Drag.active: appItemMouseArea.drag.active
                            Drag.hotSpot.x: width / 2
                            Drag.hotSpot.y: height / 2
                            MouseArea {
                                id: appItemMouseArea
                                anchors.centerIn: parent
                                hoverEnabled: true
                                width: 170; height: width
                                acceptedButtons: Qt.LeftButton | Qt.RightButton
                                ToolTip.delay: 500
                                ToolTip.text: name
                                ToolTip.visible: iconText.truncated && containsMouse
                                LingmoUIItems.StyleBackground {
                                    id: controlBase
                                    anchors.fill: parent
                                    useStyleTransparency: false
                                    paletteRole: Platform.Theme.Light
                                    radius: 16
                                    alpha: appItem.isSelect ? 0.00 : parent.containsPress ? 0.25 : parent.containsMouse ? 0.15 : 0.00
                                    Item {
                                        anchors.top: parent.top
                                        anchors.topMargin: 14
                                        anchors.horizontalCenter: parent.horizontalCenter
                                        height: 108
                                        width: 108

                                        LingmoUIItems.StyleBackground {
                                            id: imageBase
                                            anchors.fill: parent
                                            paletteRole: Platform.Theme.Text
                                            useStyleTransparency: false
                                            alpha: 0.25
                                            radius: 24
                                            visible: false
                                        }
                                        Image {
                                            id: iconImage
                                            anchors.centerIn: parent
                                            height: 96
                                            width: 96
                                            source: icon
                                            cache: false
                                        }
                                    }

                                    LingmoUIItems.StyleText {
                                        id: iconText
                                        visible: !appItem.isSelect
                                        width: parent.width
                                        horizontalAlignment: Text.AlignHCenter
                                        anchors.bottom: parent.bottom
                                        anchors.bottomMargin: 20
                                        anchors.horizontalCenter: parent.horizontalCenter
                                        text: name
                                        elide: Text.ElideRight
                                        paletteRole: Platform.Theme.HighlightedText
                                    }
                                }

                                onClicked: {
                                    if (mouse.button === Qt.RightButton) {
                                        appGridView.model.openMenu(index, MenuInfo.FullScreen);
                                        return;
                                    }
                                    if (mouse.button === Qt.LeftButton) {
                                        appGridView.model.appClicked(index);
                                        return;
                                    }
                                }
                                onPressAndHold: {
                                    if (mouse.button === Qt.LeftButton) {
                                        originalX = appItem.x;
                                        originalY = appItem.y;
                                        appItem.x = appItem.mapToItem(contentViewBase,0,0).x;
                                        appItem.y = appItem.mapToItem(contentViewBase,0,0).y;
                                        drag.target = appItem;
                                        appItem.parent = contentViewBase;
                                        appItem.isSelect = true;
                                        appGridView.selectId = id;
                                    }
                                }
                                onReleased: {
                                    parent.Drag.drop();
                                    appItem.isSelect = false;
                                    drag.target = null;
                                    appItem.parent = dropArea;
                                    appItem.x = originalX;
                                    appItem.y = originalY;
                                }
                            }
                        }
                    }
                }

                Component {
                    id: normalFolderComponent
                    DropArea {
                        onEntered: {
                            folderItem.isSelect = true;
                        }
                        onExited: {
                            folderItem.isSelect = false;
                        }
                        onDropped: {
                            appGridView.model.addAppToFolder(appGridView.selectId, id);
                        }
                        MouseArea {
                            anchors.centerIn: parent
                            hoverEnabled: true
                            width: 170; height: width
                            acceptedButtons: Qt.LeftButton | Qt.RightButton
                            ToolTip.delay: 500
                            ToolTip.text: name
                            ToolTip.visible: folderText.truncated && containsMouse
                            LingmoUIItems.StyleBackground {
                                anchors.fill: parent
                                useStyleTransparency: false
                                paletteRole: Platform.Theme.Light
                                radius: 16
                                alpha: parent.containsPress ? 0.25 : parent.containsMouse ? 0.15 : 0.00

                                Item {
                                    id: folderItem
                                    property bool isSelect: false
                                    anchors.horizontalCenter: parent.horizontalCenter
                                    height: 108; width: 108
                                    anchors.top: parent.top
                                    anchors.topMargin: 14

                                    LingmoUIItems.StyleBackground {
                                        anchors.fill: parent
                                        paletteRole: Platform.Theme.Text
                                        useStyleTransparency: false
                                        alpha: 0.25
                                        radius: 24
                                        visible: folderItem.isSelect
                                    }

                                    AppControls2.FolderIcon {
                                        id: folderIcon
                                        width: 86
                                        height: 86
                                        rows: 4; columns: 4
                                        spacing: 2; padding: 8
                                        icons: icon
                                        radius: 16; alpha: folderItem.isSelect ? 0 : 0.25
                                        anchors.centerIn: parent
                                    }
                                }

                                LingmoUIItems.StyleText {
                                    id: folderText
                                    anchors.bottom: parent.bottom
                                    anchors.bottomMargin: 20
                                    width: parent.width
                                    horizontalAlignment: Text.AlignHCenter
                                    anchors.horizontalCenter: parent.horizontalCenter
                                    elide: Text.ElideRight
                                    text: name
                                    paletteRole: Platform.Theme.HighlightedText
                                }
                            }

                            onClicked: {
                                if (mouse.button === Qt.RightButton) {
                                    modelManager.getAppModel().openMenu(index, MenuInfo.FullScreen);
                                    return;
                                }
                                if (mouse.button === Qt.LeftButton) {
                                    var x = folderIcon.mapToGlobal(0,0).x;
                                    var y = folderIcon.mapToGlobal(0,0).y
                                    openFolderSignal(id, name, x, y);
                                    // 执行隐藏动画，并且当前图标消失且鼠标区域不可用
                                    root.isContentShow = false;
                                    folderIcon.opacity = 0;
                                    enabled = false;
                                    hoverEnabled = false;
                                    return;
                                }
                            }
                            function resetOpacity() {
                                folderIcon.opacity = 1;
                                enabled = true;
                                hoverEnabled = true;
                            }
                            Component.onCompleted: root.contentShowFinished.connect(resetOpacity)
                            Component.onDestruction: root.contentShowFinished.disconnect(resetOpacity)
                        }
                    }
                }
            }
        }

        // view id: 2
        Component {
            id: labelViewComponent
            Flickable {
                id: labelViewFlickable
                ScrollBar.vertical: fullScreenScrollBar
                contentHeight: labelColumn.height
                flickableDirection: Flickable.VerticalFlick
                boundsBehavior: Flickable.StopAtBounds
                interactive: !contentYAnimation.running

                Column {
                    id: labelColumn
                    width: parent.width
                    height: childrenRect.height
                    Repeater {
                        id: labelRepeater
                        model: modelManager.getLabelGroupModel()
                        delegate: GridView {
                            id: labelAppsGridView
                            width: parent.width
                            height: contentHeight

                            property int labelIndex: index

                            interactive: false
                            cacheBuffer: 50
                            cellWidth: contentViewBase.cellWidth
                            cellHeight: contentViewBase.cellHeight

                            header: Item {
                                width: labelAppsGridView.width
                                height: contentViewBase.headerHeight
                                Row {
                                    anchors.fill: parent
                                    anchors.leftMargin: 67
                                    spacing: 15
                                    LingmoUIItems.StyleText {
                                        id: labelName
                                        anchors.verticalCenter: parent.verticalCenter
                                        verticalAlignment: Text.AlignVCenter
                                        width: contentWidth
                                        text: name
                                        paletteRole: Platform.Theme.HighlightedText
                                    }

                                    LingmoUIItems.StyleBackground {
                                        anchors.verticalCenter: parent.verticalCenter
                                        useStyleTransparency: false
                                        alpha: 0.14
                                        paletteRole: Platform.Theme.Light
                                        height: 1
                                        width: parent.width - labelName.width - parent.spacing
                                    }
                                }
                            }

                            model: extraData
                            delegate: Item {
                                width: GridView.view.cellWidth
                                height: GridView.view.cellHeight

                                MouseArea {
                                    id: labelAppsMouseArea
                                    anchors.centerIn: parent
                                    hoverEnabled: true
                                    width: 170; height: width
                                    acceptedButtons: Qt.LeftButton | Qt.RightButton

                                    LingmoUIItems.StyleBackground {
                                        anchors.fill: parent
                                        useStyleTransparency: false
                                        paletteRole: Platform.Theme.Light
                                        radius: Platform.Theme.maxRadius
                                        alpha: parent.containsPress ? 0.25 : parent.containsMouse ? 0.15 : 0.00
                                        AppControls2.IconLabel {
                                            anchors.fill: parent
                                            iconWidth: 96; iconHeight: 96
                                            appName: modelData.name
                                            appIcon: modelData.icon
                                            spacing: 8
                                            textHighLight: true
                                            ToolTip.delay: 500
                                            ToolTip.text: modelData.name
                                            ToolTip.visible: textTruncated && labelAppsMouseArea.containsMouse
                                        }
                                    }

                                    onClicked: function (mouse) {
                                        if (mouse.button === Qt.RightButton) {
                                            labelRepeater.model.openMenu(labelAppsGridView.labelIndex, model.index);
                                            return;
                                        }
                                        if (mouse.button === Qt.LeftButton) {
                                            labelRepeater.model.openApp(labelAppsGridView.labelIndex, model.index);
                                            return;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                onContentYChanged: {
                    if (contentYAnimation.running) {
                        return
                    }
                    if ((contentY + height) === contentHeight) {
                        labelListView.currentIndex = labelRepeater.count - 1
                        return
                    }
                    if (labelColumn.childAt(contentX,contentY) !== null) {
                        labelListView.currentIndex = labelColumn.childAt(contentX,contentY).labelIndex
                    }
                }

                NumberAnimation {
                    id: contentYAnimation
                    target: labelViewFlickable
                    property: "contentY"
                    duration: animationDuration
                    onFinished: {
                        labelViewFlickable.returnToBounds();
                    }
                }

                function scrollerView(labelId) {
                    let labelindex = labelRepeater.model.getLabelIndex(labelId);
                    if (labelindex < 0 || labelindex >= labelRepeater.count) {
                        return;
                    }

                    let nextY = labelRepeater.itemAt(labelindex).y;
                    let sh = labelColumn.height - nextY;
                    if (sh < height) {
                        nextY -= (height - sh);
                    }

                    contentYAnimation.running = false;
                    if (nextY === contentY) {
                        return;
                    }

                    contentYAnimation.from = contentY;
                    contentYAnimation.to = nextY;
                    contentYAnimation.running = true;
                }

                Component.onCompleted: {
                    labelListView.labelClicked.connect(scrollerView);
                }
                Component.onDestruction: {
                    labelListView.labelClicked.disconnect(scrollerView);
                }
            }
        }
    }

    Item {
        Layout.preferredWidth: 160
        Layout.fillHeight: true

        ScrollBar {
            id: fullScreenScrollBar
            anchors.verticalCenter: parent.verticalCenter
            anchors.horizontalCenter: parent.right
            anchors.horizontalCenterOffset: -24
            height: 200
            width: (hovered || pressed) ? 8 : 4
            padding: 0
            opacity: fullScreenScrollBar.size < 1.0 ? 1.0 : 0.0

            Behavior on width {
                NumberAnimation { duration: 200; easing.type: Easing.InOutQuad }
            }

            background: LingmoUIItems.StyleBackground {
                useStyleTransparency: false
                paletteRole: Platform.Theme.Dark
                alpha: 0.25
                radius: width / 2
            }

            contentItem: LingmoUIItems.StyleBackground {
                radius: width / 2
                useStyleTransparency: false
                paletteRole: Platform.Theme.Light
                alpha: fullScreenScrollBar.pressed ? 0.90 : fullScreenScrollBar.hovered ? 0.78 : 0.60
            }
        }
    }
}
