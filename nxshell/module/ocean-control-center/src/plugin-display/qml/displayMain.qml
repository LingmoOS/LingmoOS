// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick 2.15
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.15

import org.lingmo.dtk 1.0 as D

import org.lingmo.oceanui 1.0
import org.lingmo.oceanui.defApp 1.0

OceanUIObject {
    id: root
    property var screen: oceanuiData.virtualScreens[0]

    ListModel {
        id: resolutionModel
    }
    ListModel {
        id: rateModel
    }
    ListModel {
        id: fillModel
    }
    ListModel {
        id: modeModel
        ListElement {
            text: qsTr("Duplicate")
            value: "MERGE" // 1
        }
        ListElement {
            text: qsTr("Extend")
            value: "EXTEND" // 2
        }
    }
    function updateResolutionModel() {
        resolutionModel.clear()
        for (let resolution of screen.resolutionList) {
            if (resolution.width === screen.bestResolution.width && resolution.height === screen.bestResolution.height) {
                resolutionModel.insert(0, {
                                           "text": resolution.width + "×" + resolution.height + qsTr(" (Recommended)"),
                                           "value": resolution
                                       })
            } else {
                resolutionModel.append({
                                           "text": resolution.width + "×" + resolution.height,
                                           "value": resolution
                                       })
            }
        }
    }
    function updateRateModel() {
        rateModel.clear()
        for (let rate of screen.rateList) {
            if (rate === screen.bestRate) {
                rateModel.append({
                                     "text": (Math.round(rate * 100) / 100) + qsTr("Hz") + qsTr(" (Recommended)"),
                                     "value": rate
                                 })
            } else {
                rateModel.append({
                                     "text": (Math.round(rate * 100) / 100) + qsTr("Hz"),
                                     "value": rate
                                 })
            }
        }
    }
    function updateFillModel() {
        fillModel.clear()
        for (let fillmode of screen.availableFillModes) {
            switch (fillmode) {
            case "None":
                fillModel.append({
                                     "text": qsTr("Default"),
                                     "icon": "Default",
                                     "value": fillmode
                                 })
                break
            case "Full aspect":
                fillModel.append({
                                     "text": qsTr("Fit"),
                                     "icon": "Fit",
                                     "value": fillmode
                                 })
                break
            case "Full":
                fillModel.append({
                                     "text": qsTr("Stretch"),
                                     "icon": "Stretch",
                                     "value": fillmode
                                 })
                break
            case "Center":
                fillModel.append({
                                     "text": qsTr("Center"),
                                     "icon": "Center",
                                     "value": fillmode
                                 })
                break
            }
        }
    }
    function updateModeModel() {
        if (modeModel.count > 2) {
            modeModel.remove(2, modeModel.count - 2)
        }
        for (let screen of oceanuiData.screens) {
            modeModel.append({
                                 "text": qsTr("Only on %1").arg(screen.name),
                                 "value": screen.name
                             })
        }
    }
    Connections {
        target: screen
        function onResolutionListChanged() {
            updateResolutionModel()
        }
        function onRateListChanged() {
            updateRateModel()
        }
        function onAvailableFillModesChanged() {
            updateFillModel()
        }
    }
    Connections {
        target: oceanuiData
        function onScreensChanged() {
            updateModeModel()
        }
    }
    onScreenChanged: {
        updateResolutionModel()
        updateRateModel()
        updateFillModel()
    }
    Component.onCompleted: {
        updateResolutionModel()
        updateRateModel()
        updateFillModel()
        updateModeModel()
    }

    OceanUITitleObject {
        name: "multipleDisplays"
        parentName: "display"
        displayName: qsTr("Multiple Displays Settings")
        weight: 10
        visible: oceanuiData.screens.length > 1
    }
    OceanUIObject {
        name: "monitorControl"
        parentName: "display"
        weight: 20
        visible: false && oceanuiData.screens.length > 1
        pageType: OceanUIObject.Item
        page: Rectangle {
            implicitHeight: 240
            // tr("Recognize") // 识别屏幕
            Rectangle {
                x: 30
                implicitHeight: 40
                implicitWidth: 30
                color: "red"
            }
            Rectangle {
                x: 60
                implicitHeight: 40
                implicitWidth: 30
                color: "green"
            }
        }
    }
    OceanUIObject {
        name: "displayMultipleDisplays"
        parentName: "display"
        weight: 30
        visible: oceanuiData.screens.length > 1
        pageType: OceanUIObject.Item
        page: OceanUIGroupView {}
        OceanUIObject {
            name: "mode"
            parentName: "display/displayMultipleDisplays"
            displayName: qsTr("Mode")
            weight: 10
            visible: oceanuiData.screens.length > 1 && oceanuiData.isX11
            pageType: OceanUIObject.Editor
            page: ComboBox {
                textRole: "text"
                valueRole: "value"
                model: modeModel
                function indexOfMode(mode) {
                    for (var i = 0; i < model.count; i++) {
                        if (model.get(i).value === mode) {
                            return i
                        }
                    }
                    return -1
                }
                currentIndex: indexOfMode(oceanuiData.displayMode)
                onActivated: {
                    oceanuiData.displayMode = currentValue
                }
            }
        }
        OceanUIObject {
            name: "mainScreen"
            parentName: "display/displayMultipleDisplays"
            displayName: qsTr("Main Screen")
            weight: 20
            pageType: OceanUIObject.Editor
            visible: oceanuiData.virtualScreens.length > 1
            page: ComboBox {
                textRole: "name"
                model: oceanuiData.virtualScreens
                function indexOfScreen(primary) {
                    for (var i = 0; i < model.length; i++) {
                        if (model[i].name === primary.name) {
                            return i
                        }
                    }
                    return -1
                }

                currentIndex: oceanuiData.primaryScreen ? indexOfScreen(oceanuiData.primaryScreen) : -1
                onActivated: {
                    oceanuiData.primaryScreen = currentValue
                }
            }
        }
    }
    OceanUITitleObject {
        name: "screenTitle"
        parentName: "display"
        displayName: qsTr("Display And Layout")
        weight: 40
    }
    OceanUIObject {
        name: "screenTab"
        parentName: "display"
        weight: 50
        visible: oceanuiData.virtualScreens.length > 1
        pageType: OceanUIObject.Item
        Component {
            id: indicator
            ScreenIndicator {}
        }
        Component {
            id: itmeoutDialog
            TimeoutDialog {}
        }

        page: ScreenTab {
            model: oceanuiData.virtualScreens
            screen: root.screen
            onScreenChanged: {
                if (this.screen && this.screen !== root.screen) {
                    root.screen = this.screen
                    if (oceanuiData.isX11) {
                        indicator.createObject(this, {
                                                   "screen": getQtScreen(root.screen)
                                               }).show()
                    }
                }
            }
            function getQtScreen(screen) {
                for (var s of Qt.application.screens) {
                    if (s.virtualX === screen.x && s.virtualY === screen.y && s.width === screen.currentResolution.width && s.height === screen.currentResolution.height) {
                        return s
                    }
                }
                return null
            }
        }
    }
    OceanUIObject {
        name: "brightnessGroup"
        parentName: "display"
        displayName: qsTr("Brightness")
        weight: 60
        visible: false
        pageType: OceanUIObject.Item
        page: Rectangle {
            implicitHeight: 30
        }
    }
    OceanUIObject {
        name: "screenGroup"
        parentName: "display"
        weight: 70
        pageType: OceanUIObject.Item
        page: OceanUIGroupView {}
        OceanUIObject {
            name: "brightness"
            parentName: "display/screenGroup"
            displayName: qsTr("Brightness")
            weight: 10
            visible: false
            pageType: OceanUIObject.Editor
            page: Slider {}
        }

        OceanUIObject {
            name: "displayResolution"
            parentName: "display/screenGroup"
            displayName: qsTr("Resolution") // 分辨率
            weight: 20
            pageType: OceanUIObject.Editor
            page: ComboBox {
                model: resolutionModel
                textRole: "text"
                valueRole: "value"
                function indexOfSize(currentSize) {
                    for (var i = 0; i < model.count; i++) {
                        let v = model.get(i)
                        if (v.value.width === currentSize.width && v.value.height === currentSize.height) {
                            return i
                        }
                    }
                    return -1
                }

                currentIndex: indexOfSize(screen.currentResolution)
                onActivated: {
                    screen.currentResolution = currentValue
                    if (oceanuiData.isX11) {
                        itmeoutDialog.createObject(this, {
                                                       "screenX": root.screen.x,
                                                       "screenY": root.screen.y,
                                                       "screenWidth": root.screen.currentResolution.width,
                                                       "screenHeight": root.screen.currentResolution.height
                                                   }).show()
                    }
                }
            }
        }
        OceanUIObject {
            name: "resizeDesktop"
            parentName: "display/screenGroup"
            displayName: qsTr("Resize Desktop") // 屏幕显示
            weight: 30
            visible: false
            pageType: OceanUIObject.Editor
            page: D.ComboBox {
                id: control
                textRole: "text"
                valueRole: "value"
                // iconNameRole: "icon"
                model: fillModel
                function indexOfFill(currentFillMode) {
                    for (var i = 0; i < model.count; i++) {
                        let v = model.get(i)
                        if (v.value === currentFillMode) {
                            return i
                        }
                    }
                    return -1
                }
                currentIndex: indexOfFill(screen.currentFillMode)
                onActivated: {
                    screen.currentFillMode = currentValue
                    if (oceanuiData.isX11) {
                        itmeoutDialog.createObject(this, {
                                                       "screenX": root.screen.x,
                                                       "screenY": root.screen.y,
                                                       "screenWidth": root.screen.currentResolution.width,
                                                       "screenHeight": root.screen.currentResolution.height
                                                   }).show()
                    }
                }
            }
        }
        OceanUIObject {
            name: "displayRefreshRate"
            parentName: "display/screenGroup"
            displayName: qsTr("Refresh Rate") // 刷新率
            weight: 40
            pageType: OceanUIObject.Editor
            page: ComboBox {
                textRole: "text"
                valueRole: "value"
                model: rateModel
                function indexOfRate(currentRate) {
                    for (var i = 0; i < model.count; i++) {
                        let v = model.get(i)
                        if (v.value === currentRate) {
                            return i
                        }
                    }
                    return -1
                }
                currentIndex: indexOfRate(screen.currentRate)
                onActivated: {
                    screen.currentRate = currentValue
                    if (oceanuiData.isX11) {
                        itmeoutDialog.createObject(this, {
                                                       "screenX": root.screen.x,
                                                       "screenY": root.screen.y,
                                                       "screenWidth": root.screen.currentResolution.width,
                                                       "screenHeight": root.screen.currentResolution.height
                                                   }).show()
                    }
                }
            }
        }
        OceanUIObject {
            name: "displayRotate"
            parentName: "display/screenGroup"
            displayName: qsTr("Rotation") // 方向
            weight: 50
            visible: oceanuiData.isX11
            pageType: OceanUIObject.Editor
            page: ComboBox {
                textRole: "text"
                valueRole: "value"
                model: [{
                        "text": qsTr("Standard"),
                        "value": 1
                    }, {
                        "text": qsTr("90°"),
                        "value": 2
                    }, {
                        "text": qsTr("180°"),
                        "value": 4
                    }, {
                        "text": qsTr("270°"),
                        "value": 8
                    }]
                function indexOfRotate(rotate) {
                    for (var i = 0; i < model.length; i++) {
                        let v = model[i]
                        if (v.value === rotate) {
                            return i
                        }
                    }
                    return -1
                }

                currentIndex: indexOfRotate(screen.rotate)
                onActivated: {
                    screen.rotate = currentValue
                }
            }
        }
        OceanUIObject {
            name: "displayScaling"
            parentName: "display/screenGroup"
            displayName: qsTr("Display Scaling") //"缩放"
            weight: 60
            visible: false
            pageType: OceanUIObject.Editor
            page: ComboBox {}
            // qsTr("The monitor only supports 100% display scaling")
        }
    }
    OceanUITitleObject {
        name: "displayColorTemperature"
        parentName: "display"
        displayName: qsTr("Eye Comfort")
        weight: 80
        visible: false
    }
    OceanUIObject {
        name: "eyeComfort"
        parentName: "display"
        displayName: qsTr("Eye Comfort")
        description: qsTr("Adjust screen display to warmer colors, reducing screen blue light")
        weight: 90
        visible: false
        backgroundType: OceanUIObject.Normal
        pageType: OceanUIObject.Editor
        page: Switch {}
    }
    OceanUIObject {
        name: "eyeComfortGroup"
        parentName: "display"
        weight: 100
        visible: false
        pageType: OceanUIObject.Item
        page: OceanUIGroupView {}
        OceanUIObject {
            name: "time"
            parentName: "display/eyeComfortGroup"
            displayName: qsTr("Time")
            weight: 10
            pageType: OceanUIObject.Editor
            page: ComboBox {
                model: [qsTr("All day"), qsTr("Sunset to Sunrise"), qsTr("Custom Time")]
            }
        }
        OceanUIObject {
            name: "timeFrame"
            parentName: "display/eyeComfortGroup"
            // displayName: qsTr("自定义时间")
            weight: 20
            pageType: OceanUIObject.Editor
            page: RowLayout {
                Label {
                    text: qsTr("from")
                }
                OceanUITimeRange {// id: hourTime
                    // hour: sysItemModel.timeStart.split(":")[0]
                    // minute: sysItemModel.timeStart.split(":")[1]
                    // onTimeChanged: sysItemModel.timeStart = timeString
                }
                Label {
                    text: qsTr("to")
                }
                OceanUITimeRange {// id: hourTime
                    // hour: sysItemModel.timeStart.split(":")[0]
                    // minute: sysItemModel.timeStart.split(":")[1]
                    // onTimeChanged: sysItemModel.timeStart = timeString
                }
            }
        }
        OceanUIObject {
            name: "colorTemperature"
            parentName: "display/eyeComfortGroup"
            displayName: qsTr("Color Temperature")
            weight: 30
            pageType: OceanUIObject.Editor
            page: Slider {
                Layout.alignment: Qt.AlignVCenter
                implicitHeight: 24
            }
        }
    }
}
