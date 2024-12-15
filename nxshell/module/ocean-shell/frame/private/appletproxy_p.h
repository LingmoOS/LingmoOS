// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "appletproxy.h"

#include <dobject_p.h>
#include <QVariant>
#include <QPointer>

DS_BEGIN_NAMESPACE
/**
 * @brief
 */
class DAppletProxyPrivate : public DTK_CORE_NAMESPACE::DObjectPrivate
{
public:
    DAppletProxyPrivate(DAppletProxy *qq);
    ~DAppletProxyPrivate() override;

    D_DECLARE_PUBLIC(DAppletProxy);
};

class DAppletMetaProxyPrivate;
class DAppletMetaProxy : public DAppletProxy
{
    D_DECLARE_PRIVATE(DAppletMetaProxy)
public:
    explicit DAppletMetaProxy(QObject *meta, QObject *parent);

    virtual const QMetaObject *metaObject() const override;
    virtual void *qt_metacast(const char *clname) override;
    virtual int qt_metacall(QMetaObject::Call c, int id, void **argv) override;
};

DS_END_NAMESPACE
