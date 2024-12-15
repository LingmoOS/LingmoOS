// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "appletproxy.h"
#include "private/appletproxy_p.h"
#include "applet.h"

#include <QMetaMethod>

DS_BEGIN_NAMESPACE

DAppletProxyPrivate::DAppletProxyPrivate(DAppletProxy *qq)
    : DTK_CORE_NAMESPACE::DObjectPrivate(qq)
{
}

DAppletProxyPrivate::~DAppletProxyPrivate()
{
}

DAppletProxy::DAppletProxy(QObject *parent)
    : DAppletProxy(*new DAppletProxyPrivate(this), parent)
{
}

DAppletProxy::DAppletProxy(DAppletProxyPrivate &dd, QObject *parent)
    : QObject(parent)
    , DObject(dd)
{

}

DAppletProxy::~DAppletProxy()
{
}

class DAppletMetaProxyPrivate : public DAppletProxyPrivate
{
public:
    DAppletMetaProxyPrivate(DAppletMetaProxy *qq)
        : DAppletProxyPrivate(qq)
    {
    }
    QPointer<QObject> meta;
};

DAppletMetaProxy::DAppletMetaProxy(QObject *meta, QObject *parent)
    : DAppletProxy(*new DAppletMetaProxyPrivate(this), parent)
{
    d_func()->meta = meta;
}

const QMetaObject *DAppletMetaProxy::metaObject() const
{
    D_DC(DAppletMetaProxy);
    if (d->meta)
        return d->meta->metaObject();
    return &DAppletProxy::staticMetaObject;
}

void *DAppletMetaProxy::qt_metacast(const char *clname)
{
    D_D(DAppletMetaProxy);
    if (d->meta)
        return static_cast<void*>(const_cast<QObject*>(d->meta.data()));
    if (!clname) return nullptr;
    return DAppletProxy::qt_metacast(clname);
}

int DAppletMetaProxy::qt_metacall(QMetaObject::Call c, int id, void **argv)
{
    D_D(DAppletMetaProxy);
    if (d->meta) {
        auto _id = d->meta->qt_metacall(c, id, argv);
        if (_id >= 0)
            return _id;
    }
    return DAppletProxy::qt_metacall(c, id, argv);
}

DS_END_NAMESPACE
