/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2009, 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#ifndef WITHNODE_H
#define WITHNODE_H

#include "node.h"

using namespace KTextTemplate;

class WithNodeFactory : public AbstractNodeFactory
{
    Q_OBJECT
public:
    WithNodeFactory();

    Node *getNode(const QString &tagContent, Parser *p) const override;
};

class WithNode : public Node
{
    Q_OBJECT
public:
    WithNode(const std::vector<std::pair<QString, FilterExpression>> &namedExpressions, QObject *parent = {});

    void setNodeList(const NodeList &nodeList);

    void render(OutputStream *stream, Context *c) const override;

private:
    std::vector<std::pair<QString, FilterExpression>> m_namedExpressions;
    NodeList m_list;
};

#endif
