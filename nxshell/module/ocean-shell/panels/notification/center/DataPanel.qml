// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.lingmo.dtk 1.0
import org.lingmo.ds.notificationcenter

Control {
    id: root

    required property NotifyModel notifyModel

    contentItem: ColumnLayout {
        objectName: "dataPanel"
        RowLayout {
            Button {
                text: "open"
                onClicked: {
                    console.log("open")
                    notifyModel.open()
                }
            }
            Button {
                text: "close"
                onClicked: {
                    console.log("close")
                    notifyModel.close()
                }
            }
        }

        ComboBox {
            id: appSelector
            model: NotifyAccessor.apps
        }

        RowLayout {
            Button {
                text: "add"
                onClicked: {
                    let item = appSelector.currentValue
                    NotifyAccessor.addNotify(item, "content")
                }
            }

            Button {
                text: "remove"
            }
        }

        Label {
            text: notifyModel.dataInfo
        }

        Label {
            id: dataLabel
            text: NotifyAccessor.dataInfo
        }
    }
}
