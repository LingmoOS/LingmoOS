/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2009, 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#include "scriptablenode.h"

#include <QJSEngine>

#include "context.h"
#include "engine.h"
#include "exception.h"
#include "parser.h"
#include "scriptablecontext.h"
#include "scriptableparser.h"

ScriptableNode::ScriptableNode(QObject *parent)
    : Node(parent)
    , m_scriptEngine(nullptr)
{
}

void ScriptableNode::setScriptEngine(QJSEngine *engine)
{
    m_scriptEngine = engine;
}

void ScriptableNode::init(const QJSValue &concreteNode, const QJSValue &renderMethod)
{
    m_concreteNode = concreteNode;
    m_renderMethod = renderMethod;
}

void ScriptableNode::render(OutputStream *stream, Context *c) const
{
    ScriptableContext sc(c);
    auto contextObject = m_scriptEngine->newQObject(&sc);

    QJSValueList args;
    args << contextObject;

    // Call the render method in the context of the concreteNode
    auto value = const_cast<QJSValue &>(m_renderMethod).callWithInstance(m_concreteNode, args);

    if (!value.isError() && !value.isUndefined())
        (*stream) << value.toString();
}

ScriptableNodeFactory::ScriptableNodeFactory(QObject *parent)
    : AbstractNodeFactory(parent)
    , m_scriptEngine(nullptr)
{
}

void ScriptableNodeFactory::setScriptEngine(QJSEngine *engine)
{
    m_scriptEngine = engine;
}

void ScriptableNodeFactory::setEngine(Engine *engine)
{
    m_scriptEngine->setProperty("templateEngine", QVariant::fromValue(engine));
}

void ScriptableNodeFactory::setFactory(const QJSValue &factoryMethod)
{
    m_factoryMethod = factoryMethod;
}

Node *ScriptableNodeFactory::getNode(const QString &tagContent, Parser *p) const
{
    auto sp = new ScriptableParser(p, m_scriptEngine);
    auto parserObject = m_scriptEngine->newQObject(sp);

    QJSValueList args;
    args << tagContent;
    args << parserObject;

    auto factory = m_factoryMethod;

    auto scriptNode = factory.callWithInstance(factory, args);
    if (scriptNode.isError())
        throw KTextTemplate::Exception(TagSyntaxError, scriptNode.toString());

    auto node = qjsvalue_cast<Node *>(scriptNode);
    node->setParent(p);
    return node;
}

QJSEngine *ScriptableNode::engine()
{
    return m_scriptEngine;
}

void ScriptableNode::setNodeList(const QString &name, const QList<QObject *> &objectList)
{
    auto objectListArray = m_scriptEngine->newArray(objectList.size());

    for (auto i = 0; i < objectList.size(); ++i) {
        objectListArray.setProperty(i, m_scriptEngine->newQObject(objectList.at(i)));
    }
    m_concreteNode.setProperty(name, objectListArray);
}

#include "moc_scriptablenode.cpp"
