/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2009, 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#include "now.h"

#include "../lib/exception.h"
#include "parser.h"

#include <QDateTime>

NowNodeFactory::NowNodeFactory() = default;

Node *NowNodeFactory::getNode(const QString &tagContent, Parser *p) const
{
    auto expr = tagContent.split(QLatin1Char('"'), Qt::KeepEmptyParts);

    if (expr.size() != 3) {
        throw KTextTemplate::Exception(TagSyntaxError, QStringLiteral("now tag takes one argument"));
    }

    auto formatString = expr.at(1);

    return new NowNode(formatString, p);
}

NowNode::NowNode(const QString &formatString, QObject *parent)
    : Node(parent)
    , m_formatString(formatString)
{
}

void NowNode::render(OutputStream *stream, Context *c) const
{
    Q_UNUSED(c)
    (*stream) << QDateTime::currentDateTime().toString(m_formatString);
}

#include "moc_now.cpp"
