/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2009, 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#ifndef SCRIPTABLE_FILTEREXPRESSION_H
#define SCRIPTABLE_FILTEREXPRESSION_H

#include <QObject>

#include <QJSValue>

#include "filterexpression.h"
#include "scriptablecontext.h"

class ScriptableFilterExpression : public QObject
{
    Q_OBJECT
public:
    explicit ScriptableFilterExpression(QObject *parent = {});
    ScriptableFilterExpression(QJSEngine *engine, QObject *parent = {});

    void init(const QString &content, KTextTemplate::Parser *parser);

public Q_SLOTS:
    QVariant resolve(ScriptableContext *c);

    bool isTrue(ScriptableContext *c);

    bool equals(ScriptableFilterExpression *other, ScriptableContext *scriptableC);

private:
    KTextTemplate::FilterExpression m_filterExpression;
    QJSEngine *m_engine;
};

#endif
