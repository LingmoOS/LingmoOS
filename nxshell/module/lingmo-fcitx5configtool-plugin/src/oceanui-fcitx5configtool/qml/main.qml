// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick 2.15
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15
import org.lingmo.oceanui 1.0
import org.lingmo.dtk 1.0 as D

OceanUIObject {
    // Manage input methods
    OceanUIObject {
        name: "ManageInputMethods"
        weight: 100
        pageType: OceanUIObject.Item
        page: OceanUIGroupView {
            spacing: 0
            isGroup: false
        }
        ManageInputMethodsModule {}
    }

    // Shortcuts
    OceanUIObject {
        name: "Shortcuts"
        weight: 200
        pageType: OceanUIObject.Item
        page: OceanUIGroupView {
            spacing: 0
            isGroup: false
        }
        ShortcutsModule {}
    }

    // Advanced Settings
    OceanUIObject {
        name: "AdvancedSettings"
        weight: 300
        pageType: OceanUIObject.Item
        page: OceanUIGroupView {
            spacing: 0
            isGroup: false
        }
        AdvancedSettingsModule {}
    }
}
