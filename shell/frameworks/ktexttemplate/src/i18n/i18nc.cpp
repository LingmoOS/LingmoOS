/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.0-or-later

*/

#include "i18nc.h"

#include <QStringList>

#include "abstractlocalizer.h"
#include "exception.h"
#include "parser.h"

#include <QDebug>
#include <complex>
#include <util.h>

I18ncNodeFactory::I18ncNodeFactory() = default;

Node *I18ncNodeFactory::getNode(const QString &tagContent, Parser *p) const
{
    auto expr = smartSplit(tagContent);

    if (expr.size() < 3)
        throw KTextTemplate::Exception(TagSyntaxError, QStringLiteral("Error: i18nc tag takes at least two arguments"));

    auto contextText = expr.at(1);

    if (!(contextText.startsWith(QLatin1Char('"')) && contextText.endsWith(QLatin1Char('"')))
        && !(contextText.startsWith(QLatin1Char('\'')) && contextText.endsWith(QLatin1Char('\'')))) {
        throw KTextTemplate::Exception(TagSyntaxError, QStringLiteral("Error: i18nc tag first argument must be a static string."));
    }
    contextText = contextText.mid(1, contextText.size() - 2);

    auto sourceText = expr.at(2);

    if (!(sourceText.startsWith(QLatin1Char('"')) && sourceText.endsWith(QLatin1Char('"')))
        && !(sourceText.startsWith(QLatin1Char('\'')) && sourceText.endsWith(QLatin1Char('\'')))) {
        throw KTextTemplate::Exception(TagSyntaxError, QStringLiteral("Error: i18nc tag second argument must be a static string."));
    }
    sourceText = sourceText.mid(1, sourceText.size() - 2);

    QList<FilterExpression> feList;
    for (auto i = 3; i < expr.size(); ++i) {
        feList.append(FilterExpression(expr.at(i), p));
    }

    return new I18ncNode(sourceText, contextText, feList);
}

I18ncVarNodeFactory::I18ncVarNodeFactory() = default;

KTextTemplate::Node *I18ncVarNodeFactory::getNode(const QString &tagContent, Parser *p) const
{
    auto expr = smartSplit(tagContent);

    if (expr.size() < 5)
        throw KTextTemplate::Exception(TagSyntaxError, QStringLiteral("Error: i18nc_var tag takes at least four arguments"));

    auto contextText = expr.at(1);

    if (!(contextText.startsWith(QLatin1Char('"')) && contextText.endsWith(QLatin1Char('"')))
        && !(contextText.startsWith(QLatin1Char('\'')) && contextText.endsWith(QLatin1Char('\'')))) {
        throw KTextTemplate::Exception(TagSyntaxError, QStringLiteral("Error: i18nc_var tag first argument must be a static string."));
    }
    contextText = contextText.mid(1, contextText.size() - 2);

    auto sourceText = expr.at(2);

    if (!(sourceText.startsWith(QLatin1Char('"')) && sourceText.endsWith(QLatin1Char('"')))
        && !(sourceText.startsWith(QLatin1Char('\'')) && sourceText.endsWith(QLatin1Char('\'')))) {
        throw KTextTemplate::Exception(TagSyntaxError, QStringLiteral("Error: i18nc_var tag second argument must be a static string."));
    }
    sourceText = sourceText.mid(1, sourceText.size() - 2);

    QList<FilterExpression> feList;
    for (auto i = 3; i < expr.size() - 2; ++i) {
        feList.append(FilterExpression(expr.at(i), p));
    }

    const auto resultName = expr.last();

    return new I18ncVarNode(sourceText, contextText, feList, resultName);
}

I18ncNode::I18ncNode(const QString &sourceText, const QString &context, const QList<KTextTemplate::FilterExpression> &feList, QObject *parent)
    : Node(parent)
    , m_sourceText(sourceText)
    , m_context(context)
    , m_filterExpressionList(feList)
{
}

void I18ncNode::render(OutputStream *stream, Context *c) const
{
    QVariantList args;
    for (const FilterExpression &fe : m_filterExpressionList)
        args.append(fe.resolve(c));
    const auto resultString = c->localizer()->localizeContextString(m_sourceText, m_context, args);

    streamValueInContext(stream, resultString, c);
}

I18ncVarNode::I18ncVarNode(const QString &sourceText,
                           const QString &context,
                           const QList<KTextTemplate::FilterExpression> &feList,
                           const QString &resultName,
                           QObject *parent)
    : Node(parent)
    , m_sourceText(sourceText)
    , m_context(context)
    , m_filterExpressionList(feList)
    , m_resultName(resultName)
{
}

void I18ncVarNode::render(OutputStream *stream, Context *c) const
{
    Q_UNUSED(stream)
    QVariantList args;
    for (const FilterExpression &fe : m_filterExpressionList)
        args.append(fe.resolve(c));
    const auto resultString = c->localizer()->localizeContextString(m_sourceText, m_context, args);

    c->insert(m_resultName, resultString);
}

#include "moc_i18nc.cpp"
