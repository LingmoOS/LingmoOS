/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2009, 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#ifndef REGROUPNODE_H
#define REGROUPNODE_H

#include "node.h"

using namespace KTextTemplate;

class RegroupNodeFactory : public AbstractNodeFactory
{
    Q_OBJECT
public:
    RegroupNodeFactory();

    Node *getNode(const QString &tagContent, Parser *p) const override;
};

class RegroupNode : public Node
{
    Q_OBJECT
public:
    RegroupNode(const FilterExpression &target, const FilterExpression &expression, const QString &varName, QObject *parent = {});

    void render(OutputStream *stream, Context *c) const override;

private:
    FilterExpression m_target;
    FilterExpression m_expression;
    QString m_varName;
};

#endif
