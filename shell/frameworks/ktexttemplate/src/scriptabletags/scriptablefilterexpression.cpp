/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2009, 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#include "scriptablefilterexpression.h"

#include <QJSEngine>

#include "parser.h"
#include "scriptablesafestring.h"
#include "util.h"

using namespace KTextTemplate;

ScriptableFilterExpression::ScriptableFilterExpression(QObject *parent)
    : QObject(parent)
    , m_engine(nullptr)
{
}

ScriptableFilterExpression::ScriptableFilterExpression(QJSEngine *engine, QObject *parent)
    : QObject(parent)
    , m_engine(engine)
{
}

void ScriptableFilterExpression::init(const QString &content, KTextTemplate::Parser *parser)
{
    m_filterExpression = FilterExpression(content, parser);
}

QVariant ScriptableFilterExpression::resolve(ScriptableContext *c)
{
    auto var = m_filterExpression.resolve(c->context());

    if (KTextTemplate::isSafeString(var)) {
        auto ssObj = new ScriptableSafeString(m_engine);
        ssObj->setContent(getSafeString(var));
        return m_engine->newQObject(ssObj).toVariant();
    }
    return var;
}

bool ScriptableFilterExpression::isTrue(ScriptableContext *c)
{
    return m_filterExpression.isTrue(c->context());
}

bool ScriptableFilterExpression::equals(ScriptableFilterExpression *other, ScriptableContext *scriptableC)
{
    auto c = scriptableC->context();
    return KTextTemplate::equals(m_filterExpression.resolve(c), other->m_filterExpression.resolve(c));
}

#include "moc_scriptablefilterexpression.cpp"
