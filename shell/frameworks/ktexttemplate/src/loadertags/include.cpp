/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2009, 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#include "include.h"

#include "block.h"
#include "blockcontext.h"
#include "engine.h"
#include "exception.h"
#include "parser.h"
#include "rendercontext.h"
#include "template.h"
#include "util.h"

IncludeNodeFactory::IncludeNodeFactory() = default;

Node *IncludeNodeFactory::getNode(const QString &tagContent, Parser *p) const
{
    auto expr = smartSplit(tagContent);

    if (expr.size() != 2)
        throw KTextTemplate::Exception(TagSyntaxError, QStringLiteral("Error: Include tag takes only one argument"));

    auto includeName = expr.at(1);
    auto size = includeName.size();

    if ((includeName.startsWith(QLatin1Char('"')) && includeName.endsWith(QLatin1Char('"')))
        || (includeName.startsWith(QLatin1Char('\'')) && includeName.endsWith(QLatin1Char('\'')))) {
        return new ConstantIncludeNode(includeName.mid(1, size - 2));
    }
    return new IncludeNode(FilterExpression(includeName, p), p);
}

IncludeNode::IncludeNode(const FilterExpression &fe, QObject *parent)
    : Node(parent)
    , m_filterExpression(fe)
{
}

void IncludeNode::render(OutputStream *stream, Context *c) const
{
    QString filename = getSafeString(m_filterExpression.resolve(c));

    auto ti = containerTemplate();

    auto t = ti->engine()->loadByName(filename);

    if (!t)
        throw KTextTemplate::Exception(TagSyntaxError, QStringLiteral("Template not found %1").arg(filename));

    if (t->error())
        throw KTextTemplate::Exception(t->error(), t->errorString());

    t->render(stream, c);

    if (t->error())
        throw KTextTemplate::Exception(t->error(), t->errorString());
}

ConstantIncludeNode::ConstantIncludeNode(const QString &name, QObject *parent)
    : Node(parent)
{
    m_name = name;
}

void ConstantIncludeNode::render(OutputStream *stream, Context *c) const
{
    auto ti = containerTemplate();

    auto t = ti->engine()->loadByName(m_name);
    if (!t)
        throw KTextTemplate::Exception(TagSyntaxError, QStringLiteral("Template not found %1").arg(m_name));

    if (t->error())
        throw KTextTemplate::Exception(t->error(), t->errorString());

    t->render(stream, c);

    if (t->error())
        throw KTextTemplate::Exception(t->error(), t->errorString());

    QVariant &variant = c->renderContext()->data(nullptr);
    auto blockContext = variant.value<BlockContext>();
    auto nodes = t->findChildren<BlockNode *>();
    blockContext.remove(nodes);
    variant.setValue(blockContext);
}

#include "moc_include.cpp"
