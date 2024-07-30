/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2009, 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#include "filtertag.h"

#include "../lib/exception.h"
#include "filterexpression.h"
#include "parser.h"

FilterNodeFactory::FilterNodeFactory() = default;

KTextTemplate::Node *FilterNodeFactory::getNode(const QString &tagContent, KTextTemplate::Parser *p) const
{
    auto expr = tagContent.split(QLatin1Char(' '), Qt::SkipEmptyParts);

    expr.removeFirst();

    const auto expression = expr.join(QChar::fromLatin1(' '));
    FilterExpression fe(QStringLiteral("var|%1").arg(expression), p);

    auto filters = fe.filters();
    if (filters.contains(QStringLiteral("safe")) || filters.contains(QStringLiteral("escape"))) {
        throw KTextTemplate::Exception(TagSyntaxError, QStringLiteral("Use the \"autoescape\" tag instead."));
    }

    auto n = new FilterNode(fe, p);

    auto filterNodes = p->parse(n, QStringLiteral("endfilter"));
    p->removeNextToken();

    n->setNodeList(filterNodes);
    return n;
}

FilterNode::FilterNode(const FilterExpression &fe, QObject *parent)
    : Node(parent)
    , m_fe(fe)
{
}

void FilterNode::setNodeList(const NodeList &filterList)
{
    m_filterList = filterList;
}

void FilterNode::render(OutputStream *stream, Context *c) const
{
    QString output;
    QTextStream textStream(&output);
    auto temp = stream->clone(&textStream);
    m_filterList.render(temp.data(), c);
    c->push();
    c->insert(QStringLiteral("var"), output);
    m_fe.resolve(stream, c);
    c->pop();
}

#include "moc_filtertag.cpp"
