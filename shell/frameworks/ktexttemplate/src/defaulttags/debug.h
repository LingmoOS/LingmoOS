/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2009, 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#ifndef DEBUGNODE_H
#define DEBUGNODE_H

#include "node.h"

using namespace KTextTemplate;

class DebugNodeFactory : public AbstractNodeFactory
{
    Q_OBJECT
public:
    DebugNodeFactory();

    Node *getNode(const QString &tagContent, Parser *p) const override;
};

class DebugNode : public Node
{
    Q_OBJECT
public:
    explicit DebugNode(QObject *parent = {});

    void render(OutputStream *stream, Context *c) const override;

private:
    FilterExpression m_filterExpression;
    QString m_name;
    NodeList m_list;
};

#endif
