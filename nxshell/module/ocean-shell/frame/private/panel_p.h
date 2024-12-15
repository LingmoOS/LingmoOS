// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "containment_p.h"
#include "panel.h"
#include "qmlengine.h"

#include <dobject_p.h>
#include <QVariant>

DS_BEGIN_NAMESPACE
/**
 * @brief 插件项
 */
class DPanelPrivate : public DContainmentPrivate
{
public:
    explicit DPanelPrivate(DPanel *qq)
        : DContainmentPrivate(qq)
    {

    }

    void initDciSearchPaths();
    void ensurePopupWindow() const;
    void ensureToolTipWindow() const;
    void ensureMenuWindow() const;

    DQmlEngine *m_engine = nullptr;
    QQuickWindow *m_popupWindow = nullptr;
    QQuickWindow *m_toolTipWindow = nullptr;
    QQuickWindow *m_menuWindow = nullptr;

    D_DECLARE_PUBLIC(DPanel)
};

DS_END_NAMESPACE

