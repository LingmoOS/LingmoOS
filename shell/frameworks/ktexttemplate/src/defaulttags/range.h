/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2009, 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#ifndef RANGE_H
#define RANGE_H

#include "node.h"

using namespace KTextTemplate;

class RangeNodeFactory : public AbstractNodeFactory
{
public:
    RangeNodeFactory();

    Node *getNode(const QString &tagContent, Parser *p) const override;
};

class RangeNode : public Node
{
    Q_OBJECT
public:
    RangeNode(const QString &name, const FilterExpression &startExpression, const FilterExpression &stopExpression, QObject *parent = {});
    RangeNode(const QString &name,
              const FilterExpression &startExpression,
              const FilterExpression &stopExpression,
              const FilterExpression &stepExpression = {},
              QObject *parent = {});

    void setNodeList(const NodeList &list);

    void render(OutputStream *stream, Context *c) const override;

private:
    NodeList m_list;
    QString m_name;
    FilterExpression m_startExpression;
    FilterExpression m_stopExpression;
    FilterExpression m_stepExpression;
};

#endif
