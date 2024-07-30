/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2009, 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#ifndef SCRIPTABLE_NODE_H
#define SCRIPTABLE_NODE_H

#include <QJSValue>

#include "node.h"

class QJSEngine;

namespace KTextTemplate
{
class Context;
class Engine;
}

using namespace KTextTemplate;

class ScriptableNode : public Node
{
    Q_OBJECT
public:
    explicit ScriptableNode(QObject *parent = {});
    void setScriptEngine(QJSEngine *engine);
    void init(const QJSValue &concreteNode, const QJSValue &renderMethod);

    QJSEngine *engine();

    void render(OutputStream *stream, Context *c) const override;

private:
    QJSEngine *m_scriptEngine;
    QJSValue m_concreteNode;
    QJSValue m_renderMethod;

public Q_SLOTS:
    void setNodeList(const QString &name, const QList<QObject *> &);
};

class ScriptableNodeFactory : public AbstractNodeFactory
{
    Q_OBJECT
public:
    ScriptableNodeFactory(QObject *parent = {});
    void setScriptEngine(QJSEngine *engine);

    void setEngine(KTextTemplate::Engine *engine) override;
    void setFactory(const QJSValue &factoryMethod);

    Node *getNode(const QString &tagContent, Parser *p = {}) const override;

private:
    QJSEngine *m_scriptEngine;
    QJSValue m_factoryMethod;
};

#endif
