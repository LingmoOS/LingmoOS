/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2009, 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#include "debug.h"

#include "parser.h"

DebugNodeFactory::DebugNodeFactory() = default;

Node *DebugNodeFactory::getNode(const QString &tagContent, Parser *p) const
{
    Q_UNUSED(tagContent)
    return new DebugNode(p);
}

DebugNode::DebugNode(QObject *parent)
    : Node(parent)
{
}

void DebugNode::render(OutputStream *stream, Context *c) const
{
    QString ret;
    auto i = 0;
    auto h = c->stackHash(i++);

    ret += QStringLiteral("\n\nContext:\n");
    while (!h.isEmpty()) {
        for (auto it = h.begin(), end = h.end(); it != end; ++it) {
            ret +=
                QStringLiteral("key ") + it.key() + QStringLiteral(", ") + QStringLiteral("type ") + QLatin1String(it.value().typeName()) + QLatin1Char('\n');
        }
        h = c->stackHash(i++);
    }

    ret += QStringLiteral("End context:\n\n");

    (*stream) << ret;
}

#include "moc_debug.cpp"
