/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2009, 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#include "scriptablecontext.h"

#include "context.h"
#include "node.h"

ScriptableContext::ScriptableContext(Context *c, QObject *parent)
    : QObject(parent)
    , m_c(c)
{
}

QVariant ScriptableContext::lookup(const QString &name)
{
    return m_c->lookup(name);
}

void ScriptableContext::insert(const QString &name, const QVariant &variant)
{
    m_c->insert(name, variant);
}

void ScriptableContext::push()
{
    m_c->push();
}

void ScriptableContext::pop()
{
    m_c->pop();
}

QString ScriptableContext::render(const QList<QObject *> &list) const
{
    NodeList nodeList;
    for (auto n : list) {
        auto node = qobject_cast<Node *>(n);
        if (node)
            nodeList << node;
    }
    QString ret;
    QTextStream t(&ret);
    OutputStream stream(&t);
    nodeList.render(&stream, m_c);
    return ret;
}

#include "moc_scriptablecontext.cpp"
