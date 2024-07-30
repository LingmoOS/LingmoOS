/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#include "cachingloaderdecorator.h"

namespace KTextTemplate
{

class CachingLoaderDecoratorPrivate
{
public:
    CachingLoaderDecoratorPrivate(QSharedPointer<AbstractTemplateLoader> loader, CachingLoaderDecorator *qq)
        : q_ptr(qq)
        , m_wrappedLoader(loader)
    {
    }

    Q_DECLARE_PUBLIC(CachingLoaderDecorator)
    CachingLoaderDecorator *const q_ptr;

    const QSharedPointer<AbstractTemplateLoader> m_wrappedLoader;

    mutable QHash<QString, Template> m_cache;
};
}

using namespace KTextTemplate;

CachingLoaderDecorator::CachingLoaderDecorator(QSharedPointer<AbstractTemplateLoader> loader)
    : d_ptr(new CachingLoaderDecoratorPrivate(loader, this))
{
}

CachingLoaderDecorator::~CachingLoaderDecorator()
{
    delete d_ptr;
}

bool CachingLoaderDecorator::canLoadTemplate(const QString &name) const
{
    Q_D(const CachingLoaderDecorator);
    return d->m_wrappedLoader->canLoadTemplate(name);
}

void CachingLoaderDecorator::clear()
{
    Q_D(CachingLoaderDecorator);
    return d->m_cache.clear();
}

int CachingLoaderDecorator::size() const
{
    Q_D(const CachingLoaderDecorator);
    return d->m_cache.size();
}

bool CachingLoaderDecorator::isEmpty() const
{
    Q_D(const CachingLoaderDecorator);
    return d->m_cache.isEmpty();
}

std::pair<QString, QString> CachingLoaderDecorator::getMediaUri(const QString &fileName) const
{
    Q_D(const CachingLoaderDecorator);
    return d->m_wrappedLoader->getMediaUri(fileName);
}

Template CachingLoaderDecorator::loadByName(const QString &name, const KTextTemplate::Engine *engine) const
{
    Q_D(const CachingLoaderDecorator);
    const auto it = d->m_cache.constFind(name);
    if (it != d->m_cache.constEnd()) {
        return it.value();
    }

    const auto t = d->m_wrappedLoader->loadByName(name, engine);

    d->m_cache.insert(name, t);

    return t;
}
