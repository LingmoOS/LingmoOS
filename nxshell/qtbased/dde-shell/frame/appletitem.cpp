// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "appletitem.h"
#include "private/appletitem_p.h"
#include "applet.h"

#include <dobject_p.h>
#include <QLoggingCategory>
#include <QQmlEngine>
#include <QQmlContext>
#include <QQuickWindow>

DS_BEGIN_NAMESPACE
DCORE_USE_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(dsLog)

DAppletItem::DAppletItem(QQuickItem *parent)
    : QQuickItem(parent)
    , DObject(*new DAppletItemPrivate(this))
{
}

DAppletItem::~DAppletItem()
{
}

DApplet *DAppletItem::applet() const
{
    D_DC(DAppletItem);
    return qmlAttachedProperties(const_cast<DAppletItem *>(this));
}

DApplet *DAppletItem::qmlAttachedProperties(QObject *object)
{
    if (auto context = qmlContext(object)) {
        if (context->isValid())
            return context->contextProperty("_ds_applet").value<DApplet *>();
    }
    return nullptr;
}

DS_END_NAMESPACE
