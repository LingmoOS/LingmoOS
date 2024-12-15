// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import Qt.labs.qmlmodels
import org.lingmo.dtk 1.0
import org.lingmo.ds.notification
import org.lingmo.ds.notificationcenter

DelegateChooser {
    id: root
    required property NotifyModel notifyModel
    required property Item view

    signal setting(var pos, var params)

    role: "type"

    DelegateChoice {
        roleValue: "group"
        GroupNotify {
            id: groupNotify
            objectName: "group-" + model.appName
            width: 360
            appName: model.appName
            activeFocusOnTab: true

            Loader {
                anchors.fill: parent
                active: groupNotify.activeFocus && NotifyAccessor.debugging

                sourceComponent: FocusBoxBorder {
                    radius: groupNotify.radius
                    color: groupNotify.palette.highlight
                }
            }

            onCollapse: function () {
                console.log("collapse group", model.appName)
                notifyModel.collapseApp(index)
            }

            onSetting: function (pos) {
                let tmp = mapToItem(root.view, pos)
                root.setting(tmp, {
                                 appName: model.appName,
                                 pinned: model.pinned
                             })
            }
            onRemove: function () {
                console.log("remove group", model.appName)
                notifyModel.removeByApp(model.appName)
            }
        }
    }

    DelegateChoice {
        roleValue: "normal"
        NormalNotify {
            id: normalNotify
            objectName: "normal-" + model.appName
            width: 360
            activeFocusOnTab: true

            appName: model.appName
            iconName: model.iconName
            date: model.time
            actions: model.actions
            title: model.title
            content: model.content
            strongInteractive: model.strongInteractive
            contentIcon: model.contentIcon
            contentRowCount: model.contentRowCount
            property string defaultAction: model.defaultAction

            function invokeAction(actionId) {
                console.log("action normal", model.id, actionId)
                notifyModel.invokeAction(model.id, actionId)
            }

            Loader {
                anchors.fill: parent
                active: normalNotify.activeFocus && NotifyAccessor.debugging

                sourceComponent: FocusBoxBorder {
                    radius: normalNotify.radius
                    color: normalNotify.palette.highlight
                }
            }

            TapHandler {
                acceptedButtons: Qt.RightButton
                onTapped: function (eventPoint, button) {
                    let pos = eventPoint.position
                    setting(pos)
                }
            }

            // default action
            TapHandler {
                enabled: model.defaultAction !== ""
                acceptedButtons: Qt.LeftButton
                onTapped: invokeAction(model.defaultAction)
            }

            onSetting: function (pos) {
                let tmp = mapToItem(root.view, pos)
                root.setting(tmp, {
                                 appName: model.appName,
                                 pinned: model.pinned
                             })
            }
            onRemove: function () {
                console.log("remove normal", model.id)
                notifyModel.remove(model.id)
            }
            onActionInvoked: function (actionId) {
                invokeAction(actionId)
            }
        }
    }

    DelegateChoice {
        roleValue: "overlap"
        OverlapNotify {
            id: overlapNotify
            objectName: "overlap-" + model.appName
            focus: true
            width: 360
            activeFocusOnTab: true

            count: model.overlapCount
            appName: model.appName
            iconName: model.iconName
            date: model.time
            actions: model.actions
            title: model.title
            content: model.content
            strongInteractive: model.strongInteractive
            contentIcon: model.contentIcon
            contentRowCount: model.contentRowCount

            Loader {
                anchors.fill: parent
                active: overlapNotify.activeFocus && NotifyAccessor.debugging

                sourceComponent: FocusBoxBorder {
                    radius: overlapNotify.radius
                    color: overlapNotify.palette.highlight
                }
            }

            TapHandler {
                acceptedButtons: Qt.RightButton
                onTapped: function (eventPoint, button) {
                    let pos = eventPoint.position
                    setting(pos)
                }
            }

            onExpand: function ()
            {
                console.log("expand")
                notifyModel.expandApp(model.index)
            }
            onSetting: function (pos) {
                let tmp = mapToItem(root.view, pos)
                root.setting(tmp, {
                                 appName: model.appName,
                                 pinned: model.pinned
                             })
            }
            onRemove: function () {
                console.log("remove overlap", model.id)
                notifyModel.remove(model.id)
            }
            onActionInvoked: function (actionId) {
                console.log("action overlap", model.id, actionId)
                notifyModel.invokeAction(model.id, actionId)
            }
        }
    }
}
