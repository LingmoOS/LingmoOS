// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dtreelandwindowmanagerhelper.h"
#include "personalizationwaylandclientextension.h"

DGUI_BEGIN_NAMESPACE

TreelandWindowManagerHelper::TreelandWindowManagerHelper(QObject *parent)
    : DWindowManagerHelper(parent)
{
    connect(PersonalizationManager::instance(), &PersonalizationManager::activeChanged, this, [this](){
        Q_EMIT hasBlurWindowChanged();
        Q_EMIT hasNoTitlebarChanged();
    });
}

bool TreelandWindowManagerHelper::hasBlurWindow() const
{
    return PersonalizationManager::instance()->isSupported();
}

bool TreelandWindowManagerHelper::hasComposite() const
{
    return true;
}

bool TreelandWindowManagerHelper::hasNoTitlebar() const
{
    return PersonalizationManager::instance()->isSupported();
}

DGUI_END_NAMESPACE
