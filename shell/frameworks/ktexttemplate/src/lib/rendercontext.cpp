/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#include "rendercontext.h"

#include "node.h"

using namespace KTextTemplate;

namespace KTextTemplate
{

class RenderContextPrivate
{
    RenderContextPrivate(RenderContext *qq)
        : q_ptr(qq)
    {
    }

    Q_DECLARE_PUBLIC(RenderContext)
    RenderContext *const q_ptr;

    QList<QHash<const Node *, QVariant>> m_variantHashStack;
};
}

RenderContext::RenderContext()
    : d_ptr(new RenderContextPrivate(this))
{
}

RenderContext::~RenderContext()
{
    delete d_ptr;
}

void RenderContext::push()
{
    Q_D(RenderContext);
    d->m_variantHashStack.prepend({});
}

bool RenderContext::contains(Node *const scopeNode) const
{
    Q_D(const RenderContext);
    Q_ASSERT(!d->m_variantHashStack.isEmpty());
    return d->m_variantHashStack.last().contains(scopeNode);
}

QVariant &RenderContext::data(const Node *const scopeNode)
{
    Q_D(RenderContext);
    Q_ASSERT(!d->m_variantHashStack.isEmpty());
    return d->m_variantHashStack.last()[scopeNode];
}

void RenderContext::pop()
{
    Q_D(RenderContext);
    d->m_variantHashStack.removeFirst();
}
