// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DXCBPLATFORMINTERFACE_P_H
#define DXCBPLATFORMINTERFACE_P_H

#include "dxcbplatforminterface.h"
#include "private/dplatforminterface_p.h"

#include <QHash>
#include <DObjectPrivate>

DGUI_BEGIN_NAMESPACE

class DNativeSettings;
class DPlatformTheme;

class DXCBPlatformInterfacePrivate : public DCORE_NAMESPACE::DObjectPrivate
{
public:
    D_DECLARE_PUBLIC(DXCBPlatformInterface)
    DXCBPlatformInterfacePrivate(DXCBPlatformInterface *qq);

    void _q_onThemePropertyChanged(const QByteArray &name, const QVariant &value);

public:
    DPlatformTheme *parent = nullptr;
    bool fallbackProperty = true;
    DNativeSettings *theme;
    QHash<QString, QString> m_properties;
};

DGUI_END_NAMESPACE

#endif // DNATIVESETTINGS_P_H
