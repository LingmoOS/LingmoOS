// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package com.lingmo.assistant.services

object Constants {
    /*
        user settings
     */
    const val PREFS_KEY_SETTINGS_PORT: String = "settings_port"
    const val PREFS_KEY_SETTINGS_PASSWORD: String = "settings_password"
    const val PREFS_KEY_SETTINGS_START_ON_BOOT: String = "settings_start_on_boot"
    const val PREFS_KEY_SETTINGS_START_ON_BOOT_DELAY: String = "settings_start_on_boot_delay"
    const val PREFS_KEY_SETTINGS_SCALING: String = "settings_scaling"
    const val PREFS_KEY_SETTINGS_VIEW_ONLY: String = "settings_view_only"
    const val PREFS_KEY_SETTINGS_SHOW_POINTERS: String = "settings_show_pointers"
    const val PREFS_KEY_SETTINGS_ACCESS_KEY: String = "settings_access_key"
    const val PREFS_KEY_SETTINGS_FILE_TRANSFER: String = "settings_file_transfer"

    /*
        persisted runtime values shared between components
     */
    const val PREFS_KEY_SERVER_LAST_SCALING: String = "server_last_scaling"
    const val PREFS_KEY_INPUT_LAST_ENABLED: String = "input_last_enabled"
}
