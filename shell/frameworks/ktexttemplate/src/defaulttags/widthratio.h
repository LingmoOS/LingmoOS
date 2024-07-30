/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2009, 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#ifndef WIDTHRATIONODE_H
#define WIDTHRATIONODE_H

#include "node.h"

using namespace KTextTemplate;

class WidthRatioNodeFactory : public AbstractNodeFactory
{
    Q_OBJECT
public:
    WidthRatioNodeFactory();

    Node *getNode(const QString &tagContent, Parser *p) const override;
};

class WidthRatioNode : public Node
{
    Q_OBJECT
public:
    WidthRatioNode(const FilterExpression &valExpr, const FilterExpression &maxExpr, const FilterExpression &maxWidth, QObject *parent = {});

    void render(OutputStream *stream, Context *c) const override;

private:
    static int round(qreal);

    FilterExpression m_valExpr;
    FilterExpression m_maxExpr;
    FilterExpression m_maxWidth;
};

#endif
