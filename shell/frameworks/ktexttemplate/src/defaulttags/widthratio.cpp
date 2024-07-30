/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2009, 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#include "widthratio.h"

#include "../lib/exception.h"
#include "parser.h"

WidthRatioNodeFactory::WidthRatioNodeFactory() = default;

Node *WidthRatioNodeFactory::getNode(const QString &tagContent, Parser *p) const
{
    auto expr = smartSplit(tagContent);

    if (expr.size() != 4) {
        throw KTextTemplate::Exception(TagSyntaxError, QStringLiteral("widthratio takes three arguments"));
    }
    FilterExpression valExpr(expr.at(1), p);
    FilterExpression maxExpr(expr.at(2), p);
    FilterExpression maxWidth(expr.at(3), p);

    return new WidthRatioNode(valExpr, maxExpr, maxWidth, p);
}

WidthRatioNode::WidthRatioNode(const FilterExpression &valExpr, const FilterExpression &maxExpr, const FilterExpression &maxWidth, QObject *parent)
    : Node(parent)
    , m_valExpr(valExpr)
    , m_maxExpr(maxExpr)
    , m_maxWidth(maxWidth)
{
}

int WidthRatioNode::round(qreal number)
{
    auto intPart = (int)number;
    if (number < (intPart + 0.5))
        return intPart;
    return intPart + 1;
}

void WidthRatioNode::render(OutputStream *stream, Context *c) const
{
    auto thisVal = m_valExpr.resolve(c);
    auto maxVal = m_maxExpr.resolve(c);
    if (!thisVal.isValid() || !maxVal.isValid())
        return;

    auto tv = thisVal.value<double>();
    auto mv = maxVal.value<double>();

    if (mv == 0)
        return;

    auto maxWidth = m_maxWidth.resolve(c).value<int>();

    auto result = (tv / mv) * maxWidth;

    result = round(result);

    // TODO put integral streamers in OutputStream?
    (*stream) << QString::number(result);
}

#include "moc_widthratio.cpp"
