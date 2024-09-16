// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "widgetsinterface.h"
#include "widgetsinterface_p.h"

#include <QCoreApplication>

WIDGETS_BEGIN_NAMESPACE

#ifndef QT_DEBUG
Q_LOGGING_CATEGORY(dwLog, "dde.widgets", QtInfoMsg)
#else
Q_LOGGING_CATEGORY(dwLog, "dde.widgets")
#endif

IWidget::IWidget()
    : d(new IWidget::Private())
{
}

IWidget::~IWidget()
{
    Q_ASSERT(d);
    delete d->handler;
    d->handler = nullptr;

    delete d;
    d = nullptr;
}

WidgetHandler *IWidget::handler() const
{
    Q_ASSERT(d->handler);
    return d->handler;
}

QString IWidget::userInterfaceLanguage()
{
    return qApp->property("dapp_locale").toString();
}

WIDGETS_END_NAMESPACE
