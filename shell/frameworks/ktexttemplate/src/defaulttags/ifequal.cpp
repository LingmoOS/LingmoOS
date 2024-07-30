/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2009, 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#include "ifequal.h"

#include "../lib/exception.h"
#include "parser.h"
#include "util.h"

IfEqualNodeFactory::IfEqualNodeFactory() = default;

Node *IfEqualNodeFactory::do_getNode(const QString &tagContent, Parser *p, bool negate) const
{
    auto expr = smartSplit(tagContent);

    if (expr.size() != 3) {
        throw KTextTemplate::Exception(TagSyntaxError, QStringLiteral("%1 tag takes two arguments.").arg(expr.first()));
    }

    FilterExpression val1(expr.at(1), p);
    FilterExpression val2(expr.at(2), p);

    auto n = new IfEqualNode(val1, val2, negate, p);

    const QString endTag(QStringLiteral("end") + expr.first());
    const auto trueList = p->parse(n, {QStringLiteral("else"), endTag});
    n->setTrueList(trueList);
    NodeList falseList;
    if (p->takeNextToken().content == QStringLiteral("else")) {
        falseList = p->parse(n, endTag);
        n->setFalseList(falseList);
        p->removeNextToken();
    } // else empty falseList.

    return n;
}

Node *IfEqualNodeFactory::getNode(const QString &tagContent, Parser *p) const

{
    return do_getNode(tagContent, p, false);
}

IfNotEqualNodeFactory::IfNotEqualNodeFactory() = default;

Node *IfNotEqualNodeFactory::getNode(const QString &tagContent, Parser *p) const
{
    return do_getNode(tagContent, p, true);
}

IfEqualNode::IfEqualNode(const FilterExpression &val1, const FilterExpression &val2, bool negate, QObject *parent)
    : Node(parent)
{
    m_var1 = val1;
    m_var2 = val2;
    m_negate = negate;
}

void IfEqualNode::setTrueList(const NodeList &trueList)
{
    m_trueList = trueList;
}

void IfEqualNode::setFalseList(const NodeList &falseList)
{
    m_falseList = falseList;
}

void IfEqualNode::render(OutputStream *stream, Context *c) const
{
    auto var1 = m_var1.resolve(c);
    auto var2 = m_var2.resolve(c);

    auto equal = equals(var1, var2);

    if (((m_negate && !equal) || (!m_negate && equal)))
        m_trueList.render(stream, c);
    else
        m_falseList.render(stream, c);
}

#include "moc_ifequal.cpp"
