// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "common/accessibledefine.h"

#include "clockpanel.h"
#include "settingsview.h"
#include "zonechooseview.h"
#include "clock.h"

// 添加accessible
namespace dwclock {
SET_FORM_ACCESSIBLE(ClockView, "ClockView")
SET_FORM_ACCESSIBLE(SettingsView, "SettingsView")
SET_FORM_ACCESSIBLE(ZoneSectionView, "ZoneSectionView")
SET_FORM_ACCESSIBLE(ZoneSection, "ZoneSection")
SET_FORM_ACCESSIBLE(ZoneChooseView, "ZoneChooseView")
SET_FORM_ACCESSIBLE(ClockPanel, "ClockPanel")

QAccessibleInterface *accessibleFactory(const QString &classname, QObject *object)
{
    QAccessibleInterface *interface = nullptr;

    const QString NamespaceString("dwclock::");
    USE_ACCESSIBLE(QString(classname).replace(NamespaceString, ""), ClockView)
            ELSE_USE_ACCESSIBLE(QString(classname).replace(NamespaceString, ""), SettingsView)
            ELSE_USE_ACCESSIBLE(QString(classname).replace(NamespaceString, ""), ZoneSectionView)
            ELSE_USE_ACCESSIBLE(QString(classname).replace(NamespaceString, ""), ZoneSection)
            ELSE_USE_ACCESSIBLE(QString(classname).replace(NamespaceString, ""), ZoneChooseView)
            ELSE_USE_ACCESSIBLE(QString(classname).replace(NamespaceString, ""), ClockPanel)

    return interface;
}
}
