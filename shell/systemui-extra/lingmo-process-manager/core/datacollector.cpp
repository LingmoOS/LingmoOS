/*
 * Copyright 2023 KylinSoft Co., Ltd.
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

#include "datacollector.h"
#include <lingmosdk/diagnosetest/libkydatacollect.h>

void DataCollector::collectResouceLimitedEnebaledChanged(bool enabled)
{
    collectData({"hierarchicalFreeze", enabled ? "enabled" : "disabled"});
}

void DataCollector::collectSoftFreezeModeEnabledChanged(bool enabled)
{
    collectData({"softFreezeMode", enabled ? "enabled" : "disabled"});
}

void DataCollector::collectData(CollectData data)
{
    m_collectFuture = std::async(std::launch::async, [data]() {
        KTrackData *node = kdk_dia_data_init(KEVENTSOURCE_DESKTOP, KEVENT_CLICK);
        KCustomProperty property[1];
        property[0].key = (char *)data.key.c_str();
        property[0].value = (char *)data.value.c_str();
        kdk_dia_append_custom_property(node, property, 1);
        kdk_dia_upload_default(node, (char *)"hierarchical_freeze", (char *)"lingmo-process-manager");
        kdk_dia_data_free(node);
    });
}
