import QtQuick 2.12
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.0
import LingmoUI 1.0 as LingmoUI

Item {
    id: control

    property string url: main.url

    width: 400 + LingmoUI.Units.largeSpacing * 2
    height: _mainLayout.implicitHeight + LingmoUI.Units.largeSpacing * 2

    Rectangle {
        anchors.fill: parent
        color: LingmoUI.Theme.secondBackgroundColor
    }

    Component.onCompleted: {
        var items = mimeAppManager.recommendedApps(control.url)

        for (var i in items) {
            listView.model.append(items[i])
        }

        // 获取所有程序并添加到 allAppsModel
        var allApps = mimeAppManager.allApps(control.url)
        for (var j in allApps) {
            allAppsModel.append(allApps[j])
        }

        defaultCheckBox.checked = false
        doneButton.focus = true
    }

    function openApp() {
        if (defaultCheckBox.checked)
            mimeAppManager.setDefaultAppForFile(control.url, listView.model.get(listView.currentIndex).desktopFile)

        launcher.launchApp(listView.model.get(listView.currentIndex).desktopFile, control.url)
        main.close()
    }

    function openOtherApp() {
        if (defaultCheckBox.checked)
            mimeAppManager.setDefaultAppForFile(control.url, allAppsGridView.model.get(allAppsGridView.currentIndex).desktopFile)

        launcher.launchApp(allAppsGridView.model.get(allAppsGridView.currentIndex).desktopFile, control.url)
        main.close()
    }

    Keys.enabled: true
    Keys.onEscapePressed: main.close()

    ColumnLayout {
        id: _mainLayout
        anchors.fill: parent
        spacing: 0

        GridView {
            id: listView
            Layout.fillWidth: true
            Layout.preferredHeight: 250
            model: ListModel {}
            clip: true
            ScrollBar.vertical: ScrollBar {}

            leftMargin: LingmoUI.Units.smallSpacing
            rightMargin: LingmoUI.Units.smallSpacing

            cellHeight: {
                var extraHeight = calcExtraSpacing(80, listView.Layout.preferredHeight - topMargin - bottomMargin)
                return 80 + extraHeight
            }

            cellWidth: {
                var extraWidth = calcExtraSpacing(120, listView.width - leftMargin - rightMargin)
                return 120 + extraWidth
            }

            Label {
                anchors.centerIn: parent
                text: qsTr("No applications")
                visible: listView.count === 0
            }

            delegate: Item {
                id: item
                width: GridView.view.cellWidth
                height: GridView.view.cellHeight
                scale: mouseArea.pressed ? 0.95 : 1.0

                Behavior on scale {
                    NumberAnimation {
                        duration: 100
                    }
                }

                property bool isSelected: listView.currentIndex === index

                MouseArea {
                    id: mouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    acceptedButtons: Qt.LeftButton
                    onDoubleClicked: control.openApp()
                    onClicked: listView.currentIndex = index
                }

                Rectangle {
                    anchors.fill: parent
                    anchors.margins: LingmoUI.Units.smallSpacing
                    radius: LingmoUI.Theme.mediumRadius
                    color: isSelected ? LingmoUI.Theme.highlightColor
                                      : mouseArea.containsMouse ? Qt.rgba(LingmoUI.Theme.textColor.r,
                                                                          LingmoUI.Theme.textColor.g,
                                                                          LingmoUI.Theme.textColor.b,
                                                                          0.1) : "transparent"
                    smooth: true
                }

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: LingmoUI.Units.smallSpacing
                    spacing: LingmoUI.Units.smallSpacing

                    LingmoUI.IconItem {
                        id: icon
                        Layout.preferredHeight: 36
                        Layout.preferredWidth: height
                        source: model.icon
                        Layout.alignment: Qt.AlignHCenter
                    }

                    Label {
                        text: model.name
                        Layout.fillWidth: true
                        elide: Text.ElideMiddle
                        Layout.alignment: Qt.AlignHCenter
                        horizontalAlignment: Qt.AlignHCenter
                        color: isSelected ? LingmoUI.Theme.highlightedTextColor : LingmoUI.Theme.textColor
                    }
                }
            }
            visible: !showOtherApps
        }

        // 添加显示所有程序的 GridView
        GridView {
            id: allAppsGridView
            Layout.fillWidth: true
            Layout.preferredHeight: 250
            model: allAppsModel
            clip: true
            ScrollBar.vertical: ScrollBar {}

            leftMargin: LingmoUI.Units.smallSpacing
            rightMargin: LingmoUI.Units.smallSpacing

            cellHeight: {
                var extraHeight = calcExtraSpacing(80, allAppsGridView.Layout.preferredHeight - topMargin - bottomMargin)
                return 80 + extraHeight
            }

            cellWidth: {
                var extraWidth = calcExtraSpacing(120, allAppsGridView.width - leftMargin - rightMargin)
                return 120 + extraWidth
            }

            delegate: Item {
                id: allAppsItem
                width: GridView.view.cellWidth
                height: GridView.view.cellHeight
                scale: mouseArea.pressed ? 0.95 : 1.0

                Behavior on scale {
                    NumberAnimation {
                        duration: 100
                    }
                }

                property bool isSelected: allAppsGridView.currentIndex === index

                MouseArea {
                    id: mouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    acceptedButtons: Qt.LeftButton
                    onDoubleClicked: control.openOtherApp()
                    onClicked: allAppsGridView.currentIndex = index
                }

                Rectangle {
                    anchors.fill: parent
                    anchors.margins: LingmoUI.Units.smallSpacing
                    radius: LingmoUI.Theme.mediumRadius
                    color: isSelected ? LingmoUI.Theme.highlightColor
                                      : mouseArea.containsMouse ? Qt.rgba(LingmoUI.Theme.textColor.r,
                                                                          LingmoUI.Theme.textColor.g,
                                                                          LingmoUI.Theme.textColor.b,
                                                                          0.1) : "transparent"
                    smooth: true
                }

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: LingmoUI.Units.smallSpacing
                    spacing: LingmoUI.Units.smallSpacing

                    LingmoUI.IconItem {
                        id: icon
                        Layout.preferredHeight: 36
                        Layout.preferredWidth: height
                        source: model.icon
                        Layout.alignment: Qt.AlignHCenter
                    }

                    Label {
                        text: model.name
                        Layout.fillWidth: true
                        elide: Text.ElideMiddle
                        Layout.alignment: Qt.AlignHCenter
                        horizontalAlignment: Qt.AlignHCenter
                        color: isSelected ? LingmoUI.Theme.highlightedTextColor : LingmoUI.Theme.textColor
                    }
                }
            }
            visible: showOtherApps
        }

        CheckBox {
            id: defaultCheckBox
            focusPolicy: Qt.NoFocus
            text: qsTr("Set as default")
            enabled: listView.count >= 1
            padding: 0

            Layout.leftMargin: LingmoUI.Units.largeSpacing
            Layout.bottomMargin: LingmoUI.Units.largeSpacing
        }

        RowLayout {
            spacing: LingmoUI.Units.largeSpacing
            Layout.leftMargin: LingmoUI.Units.largeSpacing
            Layout.rightMargin: LingmoUI.Units.largeSpacing
            Layout.bottomMargin: LingmoUI.Units.largeSpacing

            Button {
                text: qsTr("Cancel")
                Layout.fillWidth: true
                onClicked: main.close()
            }

            Button {
                id: doneButton
                focus: true
                flat: true
                text: qsTr("Open")
                enabled: listView.count > 0
                Layout.fillWidth: true
                onClicked: control.openApp()
                visible: !showOtherApps
            }

            Button {
                id: doneOtherButton
                focus: true
                flat: true
                text: qsTr("Open")
                enabled: listView.count > 0
                Layout.fillWidth: true
                onClicked: control.openOtherApp()
                visible: showOtherApps
            }

            Switch {
                id: showOtherAppsSwitch
                text: qsTr("Show All Apps")
                checked: showOtherApps
                Layout.fillHeight: true
                onCheckedChanged: {
                    showOtherApps = checked
                }
            }
        }
    }

    function calcExtraSpacing(cellSize, containerSize) {
        var availableColumns = Math.floor(containerSize / cellSize)
        var extraSpacing = 0
        if (availableColumns > 0) {
            var allColumnSize = availableColumns * cellSize
            var extraSpace = Math.max(containerSize - allColumnSize, 0)
            extraSpacing = extraSpace / availableColumns
        }
        return Math.floor(extraSpacing)
    }

    // 定义一个模型用于存储所有程序
    ListModel {
        id: allAppsModel
    }

    // 控制其他应用程序的显示与隐藏
    property bool showOtherApps: false

    // onShowOtherAppsChanged: {
    //     otherAppsLabel.visible = showOtherApps
    //     allAppsGridView.visible = showOtherApps
    // }
}
