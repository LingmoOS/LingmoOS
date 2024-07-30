/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2009, 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#ifndef FIRSTOFNODE_H
#define FIRSTOFNODE_H

#include "node.h"

using namespace KTextTemplate;

class FirstOfNodeFactory : public AbstractNodeFactory
{
public:
    FirstOfNodeFactory();

    Node *getNode(const QString &tagContent, Parser *p) const override;
};

class FirstOfNode : public Node
{
    Q_OBJECT
public:
    explicit FirstOfNode(const QList<FilterExpression> &list, QObject *parent = {});

    void render(OutputStream *stream, Context *c) const override;

private:
    QList<FilterExpression> m_variableList;
};

#endif
