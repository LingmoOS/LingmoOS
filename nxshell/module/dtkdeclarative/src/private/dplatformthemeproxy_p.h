// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DPLATFORMTHEMEPROXY_P_H
#define DPLATFORMTHEMEPROXY_P_H

#include <DObjectPrivate>
#include <DObject>
#include <DPlatformTheme>

#include "dplatformthemeproxy.h"

DQUICK_BEGIN_NAMESPACE

class DPlatformThemeProxyPrivate : public DTK_CORE_NAMESPACE::DObjectPrivate
{
public:
    DPlatformThemeProxyPrivate(DPlatformThemeProxy *qq);

    DPlatformTheme *proxy;

private:
    D_DECLARE_PUBLIC(DPlatformThemeProxy)
};

DQUICK_END_NAMESPACE

#endif // DPLATFORMTHEMEPROXY_P_H
