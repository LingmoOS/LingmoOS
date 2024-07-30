/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2009, 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#ifndef SCRIPTABLE_PARSER_H
#define SCRIPTABLE_PARSER_H

#include <QJSValue>
#include <QObject>
#include <QStringList>

namespace KTextTemplate
{
class Parser;
}

using namespace KTextTemplate;

class ScriptableParser : public QObject
{
    Q_OBJECT
public:
    explicit ScriptableParser(Parser *p, QJSEngine *engine);

    Parser *parser()
    {
        return m_p;
    }

public Q_SLOTS:
    QList<QObject *> parse(QObject *parent, const QString &stopAt);
    QList<QObject *> parse(QObject *parent, const QStringList &stopAt = {});

    void skipPast(const QString &tag);

    QJSValue takeNextToken();
    bool hasNextToken() const;
    void removeNextToken();

    void loadLib(const QString &name);

private:
    Parser *m_p;
    QJSEngine *m_engine;
};

#endif
