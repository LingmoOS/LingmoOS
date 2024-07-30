/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2009, 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#ifndef IFEQUALNODE_H
#define IFEQUALNODE_H

#include "node.h"

using namespace KTextTemplate;

class IfEqualNodeFactory : public AbstractNodeFactory
{
    Q_OBJECT
public:
    IfEqualNodeFactory();

    Node *getNode(const QString &tagContent, Parser *p) const override;

protected:
    Node *do_getNode(const QString &tagContent, Parser *p, bool negate) const;
};

class IfNotEqualNodeFactory : public IfEqualNodeFactory
{
    Q_OBJECT
public:
    IfNotEqualNodeFactory();

    Node *getNode(const QString &tagContent, Parser *p) const override;
};

class IfEqualNode : public Node
{
    Q_OBJECT
public:
    IfEqualNode(const FilterExpression &val11, const FilterExpression &val2, bool negate, QObject *parent = {});

    void setTrueList(const NodeList &trueList);
    void setFalseList(const NodeList &falseList);

    void render(OutputStream *stream, Context *c) const override;

private:
    FilterExpression m_var1;
    FilterExpression m_var2;
    NodeList m_trueList;
    NodeList m_falseList;
    bool m_negate;
};

#endif
