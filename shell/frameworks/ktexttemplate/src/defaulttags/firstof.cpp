/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2009, 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#include "firstof.h"

#include "../lib/exception.h"
#include "parser.h"

FirstOfNodeFactory::FirstOfNodeFactory() = default;

Node *FirstOfNodeFactory::getNode(const QString &tagContent, Parser *p) const
{
    auto expr = smartSplit(tagContent);

    const auto tagName = expr.takeAt(0);

    if (expr.isEmpty()) {
        throw KTextTemplate::Exception(TagSyntaxError, QStringLiteral("%1 expects at least one argument").arg(tagName));
    }

    return new FirstOfNode(getFilterExpressionList(expr, p), p);
}

FirstOfNode::FirstOfNode(const QList<FilterExpression> &list, QObject *parent)
    : Node(parent)
    , m_variableList(list)
{
}

void FirstOfNode::render(OutputStream *stream, Context *c) const
{
    for (const FilterExpression &fe : m_variableList) {
        if (fe.isTrue(c)) {
            fe.resolve(stream, c);
            return;
        }
    }
}

#include "moc_firstof.cpp"
