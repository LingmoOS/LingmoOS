/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2009, 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#ifndef FILTERTAG_H
#define FILTERTAG_H

#include "node.h"

using namespace KTextTemplate;

class FilterNodeFactory : public AbstractNodeFactory
{
public:
    FilterNodeFactory();

    Node *getNode(const QString &tagContent, Parser *p) const override;
};

class FilterNode : public Node
{
    Q_OBJECT
public:
    explicit FilterNode(const FilterExpression &fe, QObject *parent = {});

    void setNodeList(const NodeList &filterList);

    void render(OutputStream *stream, Context *c) const override;

private:
    FilterExpression m_fe;
    NodeList m_filterList;
};

#endif
