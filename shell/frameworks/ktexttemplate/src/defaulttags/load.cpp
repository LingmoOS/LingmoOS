/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2009, 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#include "load.h"

#include "../lib/exception.h"
#include "parser.h"

LoadNodeFactory::LoadNodeFactory() = default;

Node *LoadNodeFactory::getNode(const QString &tagContent, Parser *p) const
{
    auto expr = tagContent.split(QLatin1Char(' '), Qt::SkipEmptyParts);

    if (expr.size() <= 1) {
        throw KTextTemplate::Exception(TagSyntaxError, QStringLiteral("%1 expects at least one argument").arg(expr.first()));
    }

    expr.takeAt(0);

    for (auto &i : expr) {
        p->loadLib(i);
    }

    return new LoadNode(p);
}

LoadNode::LoadNode(QObject *parent)
    : Node(parent)
{
}

void LoadNode::render(OutputStream *stream, Context *c) const
{
    Q_UNUSED(stream)
    Q_UNUSED(c)
}

#include "moc_load.cpp"
