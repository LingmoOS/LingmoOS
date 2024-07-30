/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#include "with_locale.h"

#include "../lib/exception.h"
#include "parser.h"
#include <util.h>

#include <QDebug>
#include <abstractlocalizer.h>

WithLocaleNodeFactory::WithLocaleNodeFactory() = default;

Node *WithLocaleNodeFactory::getNode(const QString &tagContent, Parser *p) const
{
    auto expr = smartSplit(tagContent);

    if (expr.size() != 2) {
        throw KTextTemplate::Exception(TagSyntaxError, QStringLiteral("%1 expected format is for example 'with_locale \"de_DE\"'").arg(expr.first()));
    }

    FilterExpression fe(expr.at(1), p);

    auto n = new WithLocaleNode(fe, p);
    const auto nodeList = p->parse(n, QStringLiteral("endwith_locale"));
    n->setNodeList(nodeList);
    p->removeNextToken();

    return n;
}

WithLocaleNode::WithLocaleNode(const FilterExpression &localeName, QObject *parent)
    : Node(parent)
    , m_localeName(localeName)
{
}

void WithLocaleNode::setNodeList(const NodeList &nodeList)
{
    m_list = nodeList;
}

void WithLocaleNode::render(OutputStream *stream, Context *c) const
{
    const QString name = KTextTemplate::getSafeString(m_localeName.resolve(c)).get();

    c->push();
    c->localizer()->pushLocale(name);
    m_list.render(stream, c);
    c->localizer()->popLocale();
    c->pop();
}

#include "moc_with_locale.cpp"
