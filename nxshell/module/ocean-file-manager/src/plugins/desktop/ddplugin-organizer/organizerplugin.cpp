// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "organizerplugin.h"
#include "framemanager.h"

#include <dfm-base/base/configs/dconfig/dconfigmanager.h>

DFMBASE_USE_NAMESPACE

namespace ddplugin_organizer {
DFM_LOG_REISGER_CATEGORY(DDP_ORGANIZER_NAMESPACE)

void OrganizerPlugin::initialize()
{
    QString err;
    DConfigManager::instance()->addConfig("org.lingmo.ocean.file-manager.desktop.organizer", &err);
}

bool OrganizerPlugin::start()
{
    instance = new FrameManager();
    bindEvent();

    return instance->initialize();
}

void OrganizerPlugin::stop()
{
    delete instance;
    instance = nullptr;
}

void OrganizerPlugin::bindEvent()
{
    dpfSlotChannel->connect("ddplugin_organizer", "slot_Organizer_Enabled", instance, &FrameManager::organizerEnabled);
}

}   // namespace ddplugin_organizer
