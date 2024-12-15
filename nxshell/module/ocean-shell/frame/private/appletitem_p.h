// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "appletitem.h"

#include "qmlengine.h"
#include <dobject_p.h>
#include <QVariant>

DS_BEGIN_NAMESPACE

class DAppletItemPrivate : public DTK_CORE_NAMESPACE::DObjectPrivate
{
public:
    explicit DAppletItemPrivate(DAppletItem *qq)
        : DTK_CORE_NAMESPACE::DObjectPrivate(qq)
    {
    }

    DQmlEngine *m_engine = nullptr;
};

DS_END_NAMESPACE
