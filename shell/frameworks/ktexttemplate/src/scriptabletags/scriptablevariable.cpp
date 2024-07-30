/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2009, 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#include "scriptablevariable.h"

#include <QJSEngine>

#include "scriptablesafestring.h"
#include "util.h"

ScriptableVariable::ScriptableVariable(QObject *parent)
    : QObject(parent)
    , m_engine(nullptr)
{
}

ScriptableVariable::ScriptableVariable(QJSEngine *engine, QObject *parent)
    : QObject(parent)
    , m_engine(engine)
{
}

void ScriptableVariable::setContent(const QString &content)
{
    m_variable = Variable(content);
}

QVariant ScriptableVariable::resolve(ScriptableContext *c)
{
    auto var = m_variable.resolve(c->context());

    if (KTextTemplate::isSafeString(var)) {
        auto ssObj = new ScriptableSafeString(m_engine);
        ssObj->setContent(getSafeString(var));
        return m_engine->newQObject(ssObj).toVariant();
    }
    return var;
}

bool ScriptableVariable::isTrue(ScriptableContext *c)
{
    return m_variable.isTrue(c->context());
}

#include "moc_scriptablevariable.cpp"
