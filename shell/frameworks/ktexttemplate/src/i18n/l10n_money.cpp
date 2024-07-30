/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.0-or-later

*/

#include "l10n_money.h"

#include "abstractlocalizer.h"
#include "exception.h"
#include "parser.h"
#include "util.h"

L10nMoneyNodeFactory::L10nMoneyNodeFactory() = default;

Node *L10nMoneyNodeFactory::getNode(const QString &tagContent, Parser *p) const
{
    auto expr = smartSplit(tagContent);

    if (expr.size() < 2)
        throw KTextTemplate::Exception(TagSyntaxError, QStringLiteral("Error: l10n_money tag takes at least one argument"));

    const FilterExpression value(expr.at(1), p);

    FilterExpression currency;

    if (expr.size() == 3)
        currency = FilterExpression(expr.at(2), p);

    return new L10nMoneyNode(value, currency);
}

L10nMoneyVarNodeFactory::L10nMoneyVarNodeFactory() = default;

KTextTemplate::Node *L10nMoneyVarNodeFactory::getNode(const QString &tagContent, Parser *p) const
{
    auto expr = smartSplit(tagContent);

    if (expr.size() < 4)
        throw KTextTemplate::Exception(TagSyntaxError, QStringLiteral("Error: l10n_money tag takes at least three arguments"));

    FilterExpression value(expr.at(1), p);

    FilterExpression currency;

    if (expr.size() == 3)
        currency = FilterExpression(expr.at(2), p);

    const auto resultName = expr.last();

    return new L10nMoneyVarNode(value, currency, resultName);
}

L10nMoneyNode::L10nMoneyNode(const FilterExpression &value, const FilterExpression &currency, QObject *parent)
    : Node(parent)
    , m_value(value)
    , m_currency(currency)
{
}

void L10nMoneyNode::render(OutputStream *stream, Context *c) const
{
    auto resultString = c->localizer()->localizeMonetaryValue(m_value.resolve(c).value<double>(), getSafeString(m_currency.resolve(c)).get());

    streamValueInContext(stream, resultString, c);
}

L10nMoneyVarNode::L10nMoneyVarNode(const FilterExpression &value, const FilterExpression &currency, const QString &resultName, QObject *parent)
    : Node(parent)
    , m_value(value)
    , m_currency(currency)
    , m_resultName(resultName)
{
}

void L10nMoneyVarNode::render(OutputStream *stream, Context *c) const
{
    Q_UNUSED(stream)
    const auto resultString = c->localizer()->localizeMonetaryValue(m_value.resolve(c).value<double>(), getSafeString(m_currency.resolve(c)).get());

    c->insert(m_resultName, resultString);
}

#include "moc_l10n_money.cpp"
