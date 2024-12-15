// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
import org.lingmo.oceanui 1.0

OceanUIObject {
    property string cmd
    function showPage() {
        if (cmd.length !== 0 && children.length !== 0) {
            children[0].showPage(cmd)
        }
    }
    name: "network"
    parentName: "root"
    displayName: qsTr("Network")
    icon: "oceanui_network"
    weight: 20
    onChildrenChanged: showPage()
    onActive: cmdParam => {
                  cmd = cmdParam
                  showPage()
              }
}
