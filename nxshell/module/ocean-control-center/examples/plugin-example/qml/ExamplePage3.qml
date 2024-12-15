// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick 2.15
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.15

import org.lingmo.oceanui 1.0

OceanUISettingsObject {
    id: root
    OceanUIRepeater {
        model: 13
        delegate: OceanUIObject {
            name: "item" + (index + 1)
            parentName: root.bodyUrl
            displayName: qsTr("Item") + (index + 1)
            weight: 30 + index
            backgroundType: OceanUIObject.Normal
            pageType: OceanUIObject.Editor
            page: Switch {}
        }
    }
    OceanUIObject {
        name: "delete"
        parentName: root.footerUrl
        weight: 10
        pageType: OceanUIObject.Item
        page: Button {
            text: qsTr("Delete")
        }
    }
}
