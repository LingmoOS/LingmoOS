/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2009, 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#ifndef SCRIPTABLE_CONTEXT_H
#define SCRIPTABLE_CONTEXT_H

#include <QObject>
#include <QVariant>

namespace KTextTemplate
{
class Context;
}

using namespace KTextTemplate;

class ScriptableContext : public QObject
{
    Q_OBJECT
public:
    explicit ScriptableContext(Context *c, QObject *parent = {});

    Context *context()
    {
        return m_c;
    }

public Q_SLOTS:
    QVariant lookup(const QString &name);
    void insert(const QString &name, const QVariant &variant);
    void push();
    void pop();

    QString render(const QList<QObject *> &list) const;

private:
    Context *m_c;
};

#endif
