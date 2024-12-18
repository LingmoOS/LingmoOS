// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EMBLEM_H
#define EMBLEM_H

#include "dfmplugin_emblem_global.h"

#include <dfm-framework/dpf.h>

DPEMBLEM_BEGIN_NAMESPACE

class Emblem : public dpf::Plugin
{
    Q_OBJECT

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    Q_PLUGIN_METADATA(IID "org.lingmo.plugin.common" FILE "emblem.json")
#else
    Q_PLUGIN_METADATA(IID "org.lingmo.plugin.common.qt6" FILE "emblem.json")
#endif

    DPF_EVENT_NAMESPACE(DPEMBLEM_NAMESPACE)
    DPF_EVENT_REG_SLOT(slot_FileEmblems_Paint)

    DPF_EVENT_REG_HOOK(hook_CustomEmblems_Fetch)
    DPF_EVENT_REG_HOOK(hook_ExtendEmblems_Fetch)

public:
    virtual void initialize() override;
    virtual bool start() override;
};

DPEMBLEM_END_NAMESPACE

#endif   // EMBLEM_H
