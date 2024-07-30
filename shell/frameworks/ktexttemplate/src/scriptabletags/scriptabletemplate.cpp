/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2009, 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#include "scriptabletemplate.h"

#include "context.h"
#include "engine.h"
#include "node.h"

ScriptableTemplate::ScriptableTemplate(KTextTemplate::Template t, QObject *parent)
    : QObject(parent)
    , m_template(t)
{
}

QString ScriptableTemplate::render(ScriptableContext *c) const
{
    return m_template->render(c->context());
}

QList<QObject *> ScriptableTemplate::nodeList() const
{
    auto nodeList = m_template->nodeList();
    QList<QObject *> objList;

    for (auto n : std::as_const(nodeList)) {
        objList << n;
    }
    return objList;
}

void ScriptableTemplate::setNodeList(const QList<QObject *> &list)
{
    NodeList nodeList;

    for (auto obj : list) {
        auto n = qobject_cast<Node *>(obj);
        if (n) {
            nodeList << n;
        }
    }
    m_template->setNodeList(nodeList);
}

#include "moc_scriptabletemplate.cpp"
