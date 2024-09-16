// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DQMLGLOBALOBJECT_P_P_H
#define DQMLGLOBALOBJECT_P_P_H

#include <DObjectPrivate>
#include <DObject>
#include <DWindowManagerHelper>

#include "dqmlglobalobject_p.h"

DQUICK_BEGIN_NAMESPACE

class DQMLGlobalObjectPrivate : public DTK_CORE_NAMESPACE::DObjectPrivate
{
public:
    DQMLGlobalObjectPrivate(DQMLGlobalObject *qq);

    void ensurePalette();
    void updatePalettes();
    void _q_onPaletteChanged();
    void ensureWebsiteInfo();

    mutable DPlatformThemeProxy *platformTheme = nullptr;

    bool paletteInit = false;
    QPalette palette;
    QPalette inactivePalette;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QQuickPalette *quickPalette = nullptr;
    QQuickPalette *inactiveQuickPalette = nullptr;
#endif
    QString deepinWebsiteName;
    QString deepinWebsiteLink;
    QString deepinDistributionOrgLogo;

private:
    D_DECLARE_PUBLIC(DQMLGlobalObject)
};

DQUICK_END_NAMESPACE

#endif // DQMLGLOBALOBJECT_P_P_H
