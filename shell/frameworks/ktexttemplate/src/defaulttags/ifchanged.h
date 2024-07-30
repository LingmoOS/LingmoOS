/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2009, 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#ifndef IFCHANGEDNODE_H
#define IFCHANGEDNODE_H

#include "node.h"

using namespace KTextTemplate;

class IfChangedNodeFactory : public AbstractNodeFactory
{
    Q_OBJECT
public:
    IfChangedNodeFactory();

    Node *getNode(const QString &tagContent, Parser *p) const override;
};

class IfChangedNode : public Node
{
    Q_OBJECT
public:
    explicit IfChangedNode(const QList<FilterExpression> &feList, QObject *parent = {});

    void setTrueList(const NodeList &trueList);
    void setFalseList(const NodeList &falseList);

    void render(OutputStream *stream, Context *c) const override;

private:
    NodeList m_trueList;
    NodeList m_falseList;
    QList<FilterExpression> m_filterExpressions;
    mutable QVariant m_lastSeen;
    QString m_id;
};

#endif
