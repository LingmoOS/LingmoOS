/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2009, 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#ifndef SCRIPTABLE_VARIABLE_H
#define SCRIPTABLE_VARIABLE_H

#include <QJSValue>
#include <QObject>

#include "scriptablecontext.h"
#include "variable.h"

using namespace KTextTemplate;

class ScriptableVariable : public QObject
{
    Q_OBJECT
public:
    ScriptableVariable(QObject *parent = {});
    ScriptableVariable(QJSEngine *engine, QObject *parent = {});

    void setContent(const QString &content);

public Q_SLOTS:
    QVariant resolve(ScriptableContext *c);

    bool isTrue(ScriptableContext *c);

private:
    Variable m_variable;
    QJSEngine *m_engine;
};

#endif
