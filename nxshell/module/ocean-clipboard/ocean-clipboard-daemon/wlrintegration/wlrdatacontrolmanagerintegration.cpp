// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "wlrdatacontrolmanagerintegration.h"

WlrDataControlManagerIntegration::WlrDataControlManagerIntegration()
    : WaylandClientExtensionTemplate<WlrDataControlManagerIntegration>(2)
{

}

WlrDataControlManagerIntegration::~WlrDataControlManagerIntegration()
{
    // Do not destroy the global object
}
