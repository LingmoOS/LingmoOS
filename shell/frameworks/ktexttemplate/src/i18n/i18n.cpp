/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.0-or-later

*/

#include "i18n.h"

#include <QStringList>

#include "abstractlocalizer.h"
#include "exception.h"
#include "parser.h"

#include <QDebug>
#include <complex>
#include <util.h>

I18nNodeFactory::I18nNodeFactory() = default;

Node *I18nNodeFactory::getNode(const QString &tagContent, Parser *p) const
{
    const auto expr = smartSplit(tagContent);

    if (expr.size() < 2)
        throw KTextTemplate::Exception(TagSyntaxError, QStringLiteral("Error: i18n tag takes at least one argument"));

    auto sourceText = expr.at(1);
    const auto size = sourceText.size();

    if (!(sourceText.startsWith(QLatin1Char('"')) && sourceText.endsWith(QLatin1Char('"')))
        && !(sourceText.startsWith(QLatin1Char('\'')) && sourceText.endsWith(QLatin1Char('\'')))) {
        throw KTextTemplate::Exception(TagSyntaxError, QStringLiteral("Error: i18n tag first argument must be a static string."));
    }
    sourceText = sourceText.mid(1, size - 2);

    QList<FilterExpression> feList;
    for (auto i = 2; i < expr.size(); ++i) {
        feList.append(FilterExpression(expr.at(i), p));
    }

    return new I18nNode(sourceText, feList);
}

I18nVarNodeFactory::I18nVarNodeFactory() = default;

KTextTemplate::Node *I18nVarNodeFactory::getNode(const QString &tagContent, Parser *p) const
{
    auto expr = smartSplit(tagContent);

    if (expr.size() < 4)
        throw KTextTemplate::Exception(TagSyntaxError, QStringLiteral("Error: i18n_var tag takes at least three arguments"));

    auto sourceText = expr.at(1);
    const auto size = sourceText.size();

    if (!(sourceText.startsWith(QLatin1Char('"')) && sourceText.endsWith(QLatin1Char('"')))
        && !(sourceText.startsWith(QLatin1Char('\'')) && sourceText.endsWith(QLatin1Char('\'')))) {
        throw KTextTemplate::Exception(TagSyntaxError, QStringLiteral("Error: i18n tag first argument must be a static string."));
    }
    sourceText = sourceText.mid(1, size - 2);

    QList<FilterExpression> feList;
    for (auto i = 2; i < expr.size() - 2; ++i) {
        feList.append(FilterExpression(expr.at(i), p));
    }

    auto resultName = expr.last();

    return new I18nVarNode(sourceText, feList, resultName);
}

I18nNode::I18nNode(const QString &sourceText, const QList<KTextTemplate::FilterExpression> &feList, QObject *parent)
    : Node(parent)
    , m_sourceText(sourceText)
    , m_filterExpressionList(feList)
{
}

void I18nNode::render(OutputStream *stream, Context *c) const
{
    QVariantList args;
    for (const FilterExpression &fe : m_filterExpressionList)
        args.append(fe.resolve(c));
    const auto resultString = c->localizer()->localizeString(m_sourceText, args);

    streamValueInContext(stream, resultString, c);
}

I18nVarNode::I18nVarNode(const QString &sourceText, const QList<KTextTemplate::FilterExpression> &feList, const QString &resultName, QObject *parent)
    : Node(parent)
    , m_sourceText(sourceText)
    , m_filterExpressionList(feList)
    , m_resultName(resultName)
{
}

void I18nVarNode::render(OutputStream *stream, Context *c) const
{
    Q_UNUSED(stream)
    QVariantList args;
    for (const FilterExpression &fe : m_filterExpressionList)
        args.append(fe.resolve(c));
    const auto resultString = c->localizer()->localizeString(m_sourceText, args);

    c->insert(m_resultName, resultString);
}

#include "moc_i18n.cpp"
