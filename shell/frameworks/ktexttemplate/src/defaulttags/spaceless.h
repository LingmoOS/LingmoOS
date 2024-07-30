/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2009, 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#ifndef SPACELESSNODE_H
#define SPACELESSNODE_H

#include "node.h"

using namespace KTextTemplate;

class SpacelessNodeFactory : public AbstractNodeFactory
{
    Q_OBJECT
public:
    SpacelessNodeFactory();

    Node *getNode(const QString &tagContent, Parser *p) const override;
};

class SpacelessNode : public Node
{
    Q_OBJECT
public:
    explicit SpacelessNode(QObject *parent = {});

    void setList(const NodeList &nodeList);

    void render(OutputStream *stream, Context *c) const override;

private:
    static QString stripSpacesBetweenTags(const QString &input);

    NodeList m_nodeList;
};

#endif
