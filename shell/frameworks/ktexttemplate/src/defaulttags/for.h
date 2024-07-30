/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2009, 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#ifndef FORNODE_H
#define FORNODE_H

#include "node.h"

using namespace KTextTemplate;

class ForNodeFactory : public AbstractNodeFactory
{
    Q_OBJECT
public:
    ForNodeFactory();

    Node *getNode(const QString &tagContent, Parser *p) const override;
};

class ForNode : public Node
{
    Q_OBJECT
public:
    enum Reversed { IsNotReversed, IsReversed };

    ForNode(const QStringList &loopVars, const FilterExpression &fe, int reversed, QObject *parent = {});

    void setLoopList(const NodeList &loopNodeList);
    void setEmptyList(const NodeList &emptyList);

    void render(OutputStream *stream, Context *c) const override;

private:
    static void insertLoopVariables(Context *c, int listSize, int i);
    void renderLoop(OutputStream *stream, Context *c) const;

    QStringList m_loopVars;
    FilterExpression m_filterExpression;
    NodeList m_loopNodeList;
    NodeList m_emptyNodeList;
    int m_isReversed;
};

#endif
