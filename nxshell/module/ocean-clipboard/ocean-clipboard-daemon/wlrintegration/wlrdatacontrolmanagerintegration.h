// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "qwayland-wlr-data-control-unstable-v1.h"
#include "waylandclientextensiontemplate.h"

class WlrDataControlManagerIntegration : public WaylandClientExtensionTemplate<WlrDataControlManagerIntegration>,
                                         public QtWayland::zwlr_data_control_manager_v1
{
    Q_OBJECT
public:
    WlrDataControlManagerIntegration();
    ~WlrDataControlManagerIntegration() override;
};
