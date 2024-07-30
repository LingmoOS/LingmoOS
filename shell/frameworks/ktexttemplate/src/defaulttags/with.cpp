/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2009, 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#include "with.h"

#include "../lib/exception.h"
#include "parser.h"

WithNodeFactory::WithNodeFactory() = default;

Node *WithNodeFactory::getNode(const QString &tagContent, Parser *p) const
{
    auto expr = smartSplit(tagContent);
    std::vector<std::pair<QString, FilterExpression>> namedExpressions;

    if (expr.size() != 4 || expr.at(2) != QStringLiteral("as")) {
        auto newSyntax = false;
        for (int i = 1; i < expr.size(); ++i) {
            const auto parts = expr.at(i).split(QLatin1Char('='));
            if (parts.size() == 2) {
                namedExpressions.push_back({parts.at(0), FilterExpression(parts.at(1), p)});
                newSyntax = true;
            } else {
                newSyntax = false;
                break;
            }
        }

        if (!newSyntax) {
            throw KTextTemplate::Exception(TagSyntaxError, QStringLiteral("%1 expected format is 'name=value' or 'value as name'").arg(expr.first()));
        }
    } else {
        namedExpressions.push_back({expr.at(3), FilterExpression(expr.at(1), p)});
    }

    auto n = new WithNode(namedExpressions, p);
    const auto nodeList = p->parse(n, QStringLiteral("endwith"));
    n->setNodeList(nodeList);
    p->removeNextToken();

    return n;
}

WithNode::WithNode(const std::vector<std::pair<QString, FilterExpression>> &namedExpressions, QObject *parent)
    : Node(parent)
    , m_namedExpressions(namedExpressions)
{
}

void WithNode::setNodeList(const NodeList &nodeList)
{
    m_list = nodeList;
}

void WithNode::render(OutputStream *stream, Context *c) const
{
    c->push();
    for (const auto &pair : m_namedExpressions) {
        c->insert(pair.first, pair.second.resolve(c));
    }
    m_list.render(stream, c);
    c->pop();
}

#include "moc_with.cpp"
