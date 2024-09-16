// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.deepin.dtk 1.0
import org.deepin.ds.dock 1.0
import Qt.labs.platform 1.1 as LP
import Qt.labs.qmlmodels 1.2 as LQM // qml6-module-qt-labs-qmlmodels
import org.deepin.ds.dock.tray 1.0 as DDT

LQM.DelegateChooser {
    id: root

    required property int columnCount
    required property int rowCount
    required property int itemPadding
    required property var stashedSurfacePopup

    role: "delegateType"
    LQM.DelegateChoice {
        roleValue: "dummy"
        StashedItemPositioner {
            contentItem: DummyDelegate {}
        }
    }
    LQM.DelegateChoice {
        roleValue: "legacy-tray-plugin"
        StashedItemPositioner {
            id: stashItemPositioner
            contentItem: ActionLegacyTrayPluginDelegate {
                id: trayDelegate
                objectName: "stash"
                itemVisible: stashItemPositioner.itemVisible
                radius: 8

                background: AppletItemBackground {
                    radius: trayDelegate.radius
                    isActive: trayDelegate.isActive
                    backgroundColor: Palette {
                        normal {
                            common: ("transparent")
                            crystal: Qt.rgba(1.0, 1.0, 1.0, 0.40)
                        }
                        normalDark {
                            crystal: Qt.rgba(1.0, 1.0, 1.0, 0.05)
                        }
                        hovered {
                            crystal: Qt.rgba(1.0, 1.0, 1.0, 0.60)
                        }
                        hoveredDark {
                            crystal: Qt.rgba(1.0, 1.0, 1.0, 0.15)
                        }
                        pressed {
                            crystal: Qt.rgba(1.0, 1.0, 1.0, 0.50)
                        }
                        pressedDark {
                            crystal: Qt.rgba(1.0, 1.0, 1.0, 0.10)
                        }
                    }
                    insideBorderColor: Palette {
                        normal {
                            common: ("transparent")
                            crystal: Qt.rgba(1.0, 1.0, 1.0, 0.20)
                        }
                        normalDark: {
                            crystal: Qt.rgba(1.0, 1.0, 1.0, 0.05)
                        }
                        hovered: normal
                        hoveredDark: normalDark
                        pressed: hovered
                        pressedDark: normalDark
                    }
                    outsideBorderColor: Palette {
                        normal {
                            common: ("transparent")
                            crystal: Qt.rgba(0.0, 0.0, 0.0, 0.05)
                        }
                        hovered: normal
                        hoveredDark: hovered
                        pressed: hovered
                        pressedDark: pressed
                    }
                }
                Connections {
                    target: stashedSurfacePopup
                    function onPopupCreated (surfaceId)
                    {
                        if (surfaceId !== model.surfaceId)
                            return
                        // `trayDelegate.height / 2` is set in dde-tray-loader
                        stashedSurfacePopup.toolTipVOffset = Qt.binding(function () {
                            return itemPadding + trayDelegate.height / 2
                        })
                    }
                }
            }
        }
    }
    LQM.DelegateChoice {
        roleValue: "action-show-stash"
        StashedItemPositioner {
            contentItem: ActionShowStashDelegate {}
        }
    }
    LQM.DelegateChoice {
        roleValue: "action-toggle-collapse"
        StashedItemPositioner {
            contentItem: ActionToggleCollapseDelegate {
                isHorizontal: root.isHorizontal
            }
        }
    }
}
