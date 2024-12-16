// Copyright (C) 2023 JiDe Zhang <zccrs@live.com>.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Waylib.Server
import TreeLand
import TreeLand.Utils
import TreeLand.Greeter
import TreeLand.Protocols

Item {
    id: root

    Binding {
        target: Helper.cursor
        property: "themeName"
        value: PersonalizationV1.cursorTheme
    }

    Binding {
        target: Helper.cursor
        property: "size"
        value: PersonalizationV1.cursorSize
    }

    Binding {
        target: Helper.seat
        property: "keyboardFocus"
        value: Helper.activatedSurface?.surface || null
    }

    OutputRenderWindow {
        id: renderWindow

        width: Helper.outputLayout.implicitWidth
        height: Helper.outputLayout.implicitHeight

        property OutputDelegate activeOutputDelegate: null

        onOutputViewportInitialized: function (viewport) {
            // Trigger QWOutput::frame signal in order to ensure WOutputHelper::renderable
            // property is true, OutputRenderWindow when will render this output in next frame.
            Helper.enableOutput(viewport.output)
        }

        EventJunkman {
            anchors.fill: parent
            focus: false
        }

        onActiveFocusItemChanged: {
            // Focus does not return to the workspace when a popup is closed from within QML
            if (activeFocusItem === renderWindow.contentItem) {
                workspaceLoader.forceActiveFocus()
            }
        }

        Item {
            id: outputLayout
            // register backgroundImage
            property var personalizationMapper: PersonalizationV1.Attach(outputLayout)

            DynamicCreatorComponent {
                id: outputDelegateCreator
                creator: Helper.outputCreator

                OutputDelegate {
                    id: outputDelegate

                    onLastActiveCursorItemChanged: {
                        if (lastActiveCursorItem != null)
                            renderWindow.activeOutputDelegate = outputDelegate
                        else if (renderWindow.activeOutputDelegate === outputDelegate) {
                            for (const output of Helper.outputLayout.outputs) {
                                if (output.lastActiveCursorItem) {
                                    renderWindow.activeOutputDelegate = output
                                    break
                                }
                            }
                        }
                    }

                    Component.onCompleted: {
                        if (!renderWindow.activeOutputDelegate) {
                            renderWindow.activeOutputDelegate = outputDelegate
                        }
                    }

                    Component.onDestruction: {
                        if (renderWindow.activeOutputDelegate === outputDelegate) {
                            for (const output of Helper.outputLayout.outputs) {
                                if (output.lastActiveCursorItem && output !== outputDelegate) {
                                    renderWindow.activeOutputDelegate = output
                                    break
                                }
                            }
                        }
                    }
                }
            }
        }

        FocusScope {
            id: workspaceLoader
            objectName: "workspaceLoader"
            anchors.fill: parent
            enabled: visible
            focus: enabled
            ColumnLayout {
                anchors.fill: parent
                FocusScope {
                    objectName: "loadercontainer"
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    focus: true
                    activeFocusOnTab: true
                    Loader {
                        id: stackLayout
                        anchors.fill: parent
                        focus: true
                        active: true
                        sourceComponent: StackWorkspace {
                            focus: stackLayout.active
                            activeOutputDelegate: renderWindow.activeOutputDelegate
                        }
                    }

                    Loader {
                        active: !stackLayout.active
                        anchors.fill: parent
                        focus: true
                        sourceComponent: TiledWorkspace { }
                    }
                }
            }
        }

        Loader {
            active: !TreeLand.testMode
            id: greeter
            asynchronous: true
            sourceComponent: Repeater {
                model: Helper.outputLayout.outputs
                delegate: Greeter {
                    property var output: modelData.output.OutputItem.item
                    property CoordMapper outputCoordMapper: this.CoordMapper.helper.get(output)
                    focus: true
                    anchors.fill: outputCoordMapper
                }
            }
        }

        Connections {
            target: greeter.active ? GreeterModel : null
            function onAnimationPlayFinished() {
                greeter.active = false
            }
        }

        Connections {
            target: Helper
            function onGreeterVisibleChanged() {
                greeter.active = !TreeLand.testMode
            }
        }

        Shortcut {
            sequences: ["Meta+S"]
            context: Qt.ApplicationShortcut
            onActivated: {
                QmlHelper.shortcutManager.multitaskViewToggled()
            }
        }
        Shortcut {
            sequences: ["Meta+L"]
            autoRepeat: false
            context: Qt.ApplicationShortcut
            onActivated: {
                QmlHelper.shortcutManager.screenLocked()
            }
        }
        Shortcut {
            sequences: ["Alt+Right"]
            context: Qt.ApplicationShortcut
            onActivated: {
                QmlHelper.shortcutManager.nextWorkspace()
            }
        }
        Shortcut {
            sequences: ["Alt+Left"]
            context: Qt.ApplicationShortcut
            onActivated: {
                QmlHelper.shortcutManager.prevWorkspace()
            }
        }
        Shortcut {
            sequences: ["Meta+Shift+Right"]
            context: Qt.ApplicationShortcut
            onActivated: {
                QmlHelper.shortcutManager.moveToNeighborWorkspace(1)
            }
        }
        Shortcut {
            sequences: ["Meta+Shift+Left"]
            context: Qt.ApplicationShortcut
            onActivated: {
                QmlHelper.shortcutManager.moveToNeighborWorkspace(-1)
            }
        }
        Connections {
            target: QmlHelper.shortcutManager
            function onScreenLocked() {
                greeter.active = !TreeLand.testMode
            }
        }

        Component.onCompleted: {
            QmlHelper.renderWindow = this
        }
    }
}
