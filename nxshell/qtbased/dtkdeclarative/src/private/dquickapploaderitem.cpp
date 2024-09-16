// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dquickapploaderitem_p.h"

#include <DObjectPrivate>

DQUICK_BEGIN_NAMESPACE

class DQuickAppLoaderItemPrivate : public DCORE_NAMESPACE::DObjectPrivate
{
    D_DECLARE_PUBLIC(DQuickAppLoaderItem)
public:
    DQuickAppLoaderItemPrivate(DQuickAppLoaderItem *qq)
        : DObjectPrivate(qq)
        , window(nullptr)
        , loaded(false)
        , asynchronous(false)
        , progress(0)
    {

    }

    QList<QQmlComponent *> childrenComponents;
    QQuickWindow *window;
    bool loaded;
    bool asynchronous;
    qreal progress;
};

DQuickAppLoaderItem::DQuickAppLoaderItem(QQuickItem *parentItem)
    : QQuickItem(parentItem)
    , DObject(*new DQuickAppLoaderItemPrivate(this))
{

}

bool DQuickAppLoaderItem::loaded() const
{
    D_DC(DQuickAppLoaderItem);
    return d->loaded;
}

void DQuickAppLoaderItem::setLoaded(bool loaded)
{
    D_D(DQuickAppLoaderItem);
    if (loaded == d->loaded)
        return;
    d->loaded = loaded;
    Q_EMIT loadedChanged();
}

qreal DQuickAppLoaderItem::progress() const
{
    D_DC(DQuickAppLoaderItem);
    return d->progress;
}

void DQuickAppLoaderItem::setProgress(qreal progress)
{
    D_D(DQuickAppLoaderItem);
    if (qFuzzyCompare(progress, d->progress))
        return;
    d->progress = progress;
    Q_EMIT progressChanged();
}

QQuickWindow *DQuickAppLoaderItem::window() const
{
    D_DC(DQuickAppLoaderItem);
    return d->window;
}

void DQuickAppLoaderItem::setWindow(QQuickWindow *w)
{
    D_D(DQuickAppLoaderItem);
    if (w == d->window)
        return;
    d->window = w;
    Q_EMIT windowChanged();
}


bool DQuickAppLoaderItem::asynchronous() const
{
    D_DC(DQuickAppLoaderItem);
    return d->asynchronous;
}

void DQuickAppLoaderItem::setAsynchronous(bool a)
{
    D_D(DQuickAppLoaderItem);
    if (a == d->asynchronous)
        return;
    d->asynchronous = a;
    Q_EMIT asynchronousChanged();
}

DQUICK_END_NAMESPACE
