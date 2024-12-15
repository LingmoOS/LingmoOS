// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "applet_p.h"
#include "containment.h"
#include "appletitemmodel.h"

#include <dobject_p.h>
#include <QVariant>

DS_BEGIN_NAMESPACE
/**
 * @brief 插件项
 */
class DContainmentPrivate : public DAppletPrivate
{
public:
    explicit DContainmentPrivate(DContainment *qq)
        : DAppletPrivate(qq)
    {
    }
    DAppletItemModel *model() const
    {
        if (!m_model) {
            const_cast<DContainmentPrivate *>(this)->m_model = new DAppletItemModel(const_cast<DContainment *>(q_func()));
        }
        return m_model;
    }
    QList<DAppletData> groupList(const DAppletData &data) const;
    QList<DApplet *> m_applets;
    DAppletItemModel *m_model = nullptr;

    D_DECLARE_PUBLIC(DContainment)
};

DS_END_NAMESPACE
