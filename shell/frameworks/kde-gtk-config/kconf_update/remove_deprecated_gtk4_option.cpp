/*
 * SPDX-FileCopyrightText: 2021 Weng Xuetian <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "config_editor/settings_ini.h"

int main()
{
    SettingsIniEditor::unsetValue(QStringLiteral("gtk-button-images"), 4);
    SettingsIniEditor::unsetValue(QStringLiteral("gtk-menu-images"), 4);
    SettingsIniEditor::unsetValue(QStringLiteral("gtk-toolbar-style"), 4);
    return 0;
}
