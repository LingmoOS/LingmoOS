/*
 * Copyright 2024 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "settings/settings.h"

#include <gio/gio.h>

namespace aisdk
{

static const char* enabledKey = "enabled";

Settings::Settings(const char* schema) : settings_(*g_settings_new(schema)) {}

Settings::~Settings() {
    g_object_unref(&settings_);
}

bool Settings::isEnabled() const {
    return g_settings_get_boolean(&settings_, enabledKey);
}

bool Settings::setEnabled(bool enabled) {
    return g_settings_set_boolean(&settings_, enabledKey, enabled);
}

void Settings::resetEnabled() {
    g_settings_reset(&settings_, enabledKey);
}

}