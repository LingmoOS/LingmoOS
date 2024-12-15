// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include "portalwaylandcontext.h"

#include <QObject>
#include <QPointer>
#include <QDBusAbstractAdaptor>

class AbstractWaylandPortal : public QDBusAbstractAdaptor
{
public:
    AbstractWaylandPortal(PortalWaylandContext *context) : QDBusAbstractAdaptor(context), m_context(context) { }
    QPointer<PortalWaylandContext> context() { return m_context; }

private:
    QPointer<PortalWaylandContext> m_context;
};
