// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

import org.lingmo.oceanui 1.0

OceanUIObject {
    id: datetime
    // 时间和日期
    OceanUIObject {
        name: "timeAndLang"
        parentName: "system"
        displayName: qsTr("Time and date")
        description: qsTr("Time and date, time zone settings")
        icon: "oceanui_time_date"
        weight: 40
    }

    // 语言和格式
    OceanUIObject {
        name: "langAndFormat"
        parentName: "system"
        displayName: qsTr("Language and region")
        description: qsTr("System language, region format")
        icon: "oceanui_lang_format"
        weight: 45
    }

    visible: false
    OceanUIDBusInterface {
        property var locales
        service: "org.lingmo.ocean.LangSelector1"
        path: "/org/lingmo/ocean/LangSelector1"
        inter: "org.lingmo.ocean.LangSelector1"
        connection: OceanUIDBusInterface.SessionBus
        onLocalesChanged: datetime.visible = true
    }
}
