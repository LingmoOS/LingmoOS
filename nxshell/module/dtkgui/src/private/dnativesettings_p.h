// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DNATIVESETTINGS_P_H
#define DNATIVESETTINGS_P_H

#include <dobject_p.h>
#include <dtkgui_global.h>
#include <dnativesettings.h>

DGUI_BEGIN_NAMESPACE

class DNativeSettingsPrivate : public DCORE_NAMESPACE::DObjectPrivate
{
    D_DECLARE_PUBLIC(DNativeSettings)
public:
    DNativeSettingsPrivate(DNativeSettings *qq, const QByteArray &domain);
    ~DNativeSettingsPrivate();

    bool init(const QMetaObject *mo, quint32 window);

public:
    QByteArray domain;
    bool valid = false;
    QByteArrayList allKeys;
};

DGUI_END_NAMESPACE

#endif // DNATIVESETTINGS_P_H
