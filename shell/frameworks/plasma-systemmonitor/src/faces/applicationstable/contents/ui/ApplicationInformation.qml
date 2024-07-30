/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQml

QtObject {
    id: info

    property var model
    property var index
    onIndexChanged: info.update()

    property var sensorColumns
    property int role
    property int pidsRole

    property string name
    property string menuId
    property real cpu
    property real memory
    property real netInbound
    property real netOutbound
    property real diskRead
    property real diskWrite

    property string cpuMode

    property string iconName

    property var pids: []

    property var connection: Connections {
        target: model
        function onDataChanged() { if (info) { info.update() } }
    }

    function update() {
        if ( model == null || index == null) {
            return
        }

        name = "" + model.data(model.index(index.row, sensorColumns["name"]), role)
        cpu = parseFloat(model.data(model.index(index.row, sensorColumns["cpu"]), role))
        memory = parseFloat(model.data(model.index(index.row, sensorColumns["memory"]), role))
        netInbound = parseFloat(model.data(model.index(index.row, sensorColumns["netInbound"]), role))
        netOutbound = parseFloat(model.data(model.index(index.row, sensorColumns["netOutbound"]), role))
        diskRead = parseFloat(model.data(model.index(index.row, sensorColumns["diskRead"]), role))
        diskWrite = parseFloat(model.data(model.index(index.row, sensorColumns["diskWrite"]), role))
        iconName = "" + model.data(model.index(index.row, sensorColumns["iconName"]), role)

        pids = model.data(model.index(index.row, 0), pidsRole)
        if (!pids) {
            pids = []
        }
    }
}
