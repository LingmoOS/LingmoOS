/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2009, 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#include "scriptableparser.h"

#include "parser.h"

#include <QJSEngine>

ScriptableParser::ScriptableParser(KTextTemplate::Parser *p, QJSEngine *engine)
    : QObject(engine)
    , m_p(p)
    , m_engine(engine)
{
}

void ScriptableParser::removeNextToken()
{
    m_p->removeNextToken();
}

bool ScriptableParser::hasNextToken() const
{
    return m_p->hasNextToken();
}

void ScriptableParser::loadLib(const QString &name)
{
    m_p->loadLib(name);
}

QJSValue ScriptableParser::takeNextToken()
{
    Token t = m_p->takeNextToken();
    auto obj = m_engine->newObject();
    obj.setProperty(QStringLiteral("tokenType"), t.tokenType);
    obj.setProperty(QStringLiteral("content"), t.content);
    return obj;
}

void ScriptableParser::skipPast(const QString &tag)
{
    m_p->skipPast(tag);
}

QList<QObject *> ScriptableParser::parse(QObject *parent, const QString &stopAt)
{
    return parse(parent, QStringList() << stopAt);
}

QList<QObject *> ScriptableParser::parse(QObject *parent, const QStringList &stopAt)
{
    auto node = qobject_cast<Node *>(parent);
    Q_ASSERT(node);

    auto nodeList = m_p->parse(node, stopAt);
    QList<QObject *> objList;
    for (auto n : std::as_const(nodeList)) {
        objList << n;
    }
    return objList;
}

#include "moc_scriptableparser.cpp"
