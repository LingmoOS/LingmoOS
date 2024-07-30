/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#ifndef WITHLOCALENODE_H
#define WITHLOCALENODE_H

#include "node.h"

using namespace KTextTemplate;

class WithLocaleNodeFactory : public AbstractNodeFactory
{
    Q_OBJECT
public:
    WithLocaleNodeFactory();

    Node *getNode(const QString &tagContent, Parser *p) const override;
};

class WithLocaleNode : public Node
{
    Q_OBJECT
public:
    WithLocaleNode(const FilterExpression &localeName, QObject *parent = {});

    void setNodeList(const NodeList &nodeList);

    void render(OutputStream *stream, Context *c) const override;

private:
    FilterExpression m_localeName;
    NodeList m_list;
};

#endif
