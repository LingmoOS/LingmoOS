/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: iaom <zhangpengfei@kylinos.cn>
 *
 */

#include "data-collecter.h"

void DataCollecter::collectLaunchEvent(const QString &plugin, const QString &button)
{
    KCustomProperty *property = new KCustomProperty;
    property->key = strdup(QStringLiteral("pluginName").toLocal8Bit().data());
    property->value = strdup(button.toLocal8Bit().data());
    KTrackData *node = kdk_dia_data_init(KEVENTSOURCE_DESKTOP,KEVENT_CLICK);
    kdk_dia_append_custom_property(node,property,1);
    kdk_dia_upload_default(node, strdup(QStringLiteral("search_plugin_launch_event").toLocal8Bit().data()), strdup(plugin.toLocal8Bit().data()));
    kdk_dia_data_free(node);
    free(property->key);
    free(property->value);
    delete property;
}
