/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2009, 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#include "if.h"
#include "if_p.h"

#include "../lib/exception.h"
#include "parser.h"

IfNodeFactory::IfNodeFactory() = default;

Node *IfNodeFactory::getNode(const QString &tagContent, Parser *p) const
{
    auto expr = smartSplit(tagContent);

    QList<std::pair<QSharedPointer<IfToken>, NodeList>> nodelistConditions;

    auto n = new IfNode(p);

    IfParser ip(p, expr);
    auto cond = ip.parse();
    auto nodelist = p->parse(n, {QStringLiteral("elif"), QStringLiteral("else"), QStringLiteral("endif")});
    nodelistConditions.push_back(std::make_pair(cond, nodelist));

    auto token = p->takeNextToken();

    while (token.content.startsWith(QLatin1String("elif"))) {
        expr = smartSplit(token.content);

        IfParser ep(p, expr);
        cond = ep.parse();
        nodelist = p->parse(n, {QStringLiteral("elif"), QStringLiteral("else"), QStringLiteral("endif")});
        nodelistConditions.push_back(std::make_pair(cond, nodelist));

        token = p->takeNextToken();
    }
    if (token.content == QLatin1String("else")) {
        nodelist = p->parse(n, QStringLiteral("endif"));
        nodelistConditions.push_back(std::make_pair(nullptr, nodelist));
        p->takeNextToken();
    }

    n->setNodelistConditions(nodelistConditions);

    auto commandName = expr.takeAt(0);
    if (expr.size() <= 0) {
        throw KTextTemplate::Exception(TagSyntaxError, QStringLiteral("'%1' statement requires at least one argument").arg(commandName));
    }

    return n;
}

IfNode::IfNode(QObject *parent)
    : Node(parent)
{
}

void IfNode::setNodelistConditions(const QList<std::pair<QSharedPointer<IfToken>, NodeList>> &conditionNodelists)
{
    mConditionNodelists = conditionNodelists;
}

void IfNode::render(OutputStream *stream, Context *c) const
{
    // Evaluate the expression. rendering variables with the context as needed.
    // and processing nodes recursively

    for (auto &pair : mConditionNodelists) {
        bool match = false;
        if (pair.first) {
            try {
                match = KTextTemplate::variantIsTrue(pair.first->evaluate(c));
            } catch (const KTextTemplate::Exception &) {
            }
        } else {
            match = true;
        }
        if (match) {
            pair.second.render(stream, c);
            return;
        }
    }
}

#include "moc_if.cpp"
