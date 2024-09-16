// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Qt.labs.platform 1.1 as LP
import Qt.labs.qmlmodels 1.2 as LQM // qml6-module-qt-labs-qmlmodels
import org.deepin.ds.dock.tray 1.0 as DDT

LQM.DelegateChooser {
    id: root
    property bool isHorizontal: false
    property bool collapsed: false
    required property int itemPadding
    required property var surfaceAcceptor
    property bool disableInputEvents

    role: "delegateType"
    LQM.DelegateChoice {
        roleValue: "dummy"
        TrayItemPositioner {
            visualSize: dummyDelegate.visualSize
            contentItem: DummyDelegate {
                id: dummyDelegate
            }
        }
    }
    LQM.DelegateChoice {
        roleValue: "legacy-tray-plugin"
        TrayItemPositioner {
            id: traySurfacePositioner
            visualSize: traySurfaceDelegate.visualSize
            contentItem: ActionLegacyTrayPluginDelegate {
                id: traySurfaceDelegate
                objectName: "tray"
                inputEventsEnabled: !disableInputEvents && (model.sectionType !== "collapsable" || !DDT.TraySortOrderModel.collapsed)
                itemVisible: traySurfacePositioner.itemVisible
                dragable: model.sectionType !== "fixed"
                isActive: surfacePopup.isOpened

                // trayItem's popup
                DDT.TrayItemSurfacePopup {
                    id: surfacePopup
                    surfaceAcceptor: function (surfaceId) {
                        if (root.surfaceAcceptor && !root.surfaceAcceptor(surfaceId))
                            return false

                        return surfaceId === model.surfaceId
                    }
                }
            }
        }
    }
    LQM.DelegateChoice {
        roleValue: "action-show-stash"
        TrayItemPositioner {
            contentItem: ActionShowStashDelegate {}
        }
    }
    LQM.DelegateChoice {
        roleValue: "action-toggle-collapse"
        TrayItemPositioner {
            contentItem: ActionToggleCollapseDelegate {
                isHorizontal: root.isHorizontal
                inputEventsEnabled: !disableInputEvents
            }
        }
    }
    LQM.DelegateChoice {
        roleValue: "action-toggle-quick-settings"
        TrayItemPositioner {
            visualSize: Qt.size(quickSettingsDelegate.width, quickSettingsDelegate.height)
            ActionToggleQuickSettingsDelegate {
                id: quickSettingsDelegate
                isHorizontal: root.isHorizontal
            }
        }
    }
}
