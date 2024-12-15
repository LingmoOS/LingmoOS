// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <dtkdeclarative_global.h>

#include <DWindowManagerHelper>
#include <DGuiApplicationHelper>
#include <DFontManager>
#include <DPlatformTheme>
#include <DPlatformHandle>
#include <QQmlEngine>

DQUICK_BEGIN_NAMESPACE
DGUI_USE_NAMESPACE

struct WindowManagerHelperForeign
{
    Q_GADGET
    QML_FOREIGN(DWindowManagerHelper)
    QML_SINGLETON
    QML_NAMED_ELEMENT(WindowManagerHelper)
public:
    static DWindowManagerHelper *create(QQmlEngine *, QJSEngine *)
    {
        auto helper = DWindowManagerHelper::instance();
        QQmlEngine::setObjectOwnership(helper, QQmlEngine::CppOwnership);
        return helper;
    }
};

struct DGuiApplicationHelperForeign
{
    Q_GADGET
    QML_FOREIGN(DGuiApplicationHelper)
    QML_SINGLETON
    QML_NAMED_ELEMENT(ApplicationHelper)
public:
    static DGuiApplicationHelper *create(QQmlEngine *, QJSEngine *)
    {
        auto helper = DGuiApplicationHelper::instance();
        QQmlEngine::setObjectOwnership(helper, QQmlEngine::CppOwnership);
        return helper;
    }
};

struct DFontManagerForeign
{
    Q_GADGET
    QML_FOREIGN(DFontManager)
    QML_NAMED_ELEMENT(FontManager)
};

struct DPlatformHandleForeign
{
    Q_GADGET
    QML_FOREIGN(DPlatformHandle)
    QML_NAMED_ELEMENT(PlatformHandle)
};

DQUICK_END_NAMESPACE
