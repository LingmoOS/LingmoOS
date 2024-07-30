/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.0-or-later

*/

#ifndef L10N_MONEYNODE_H
#define L10N_MONEYNODE_H

#include "node.h"

namespace KTextTemplate
{
class Parser;
}

using namespace KTextTemplate;

class L10nMoneyNodeFactory : public AbstractNodeFactory
{
    Q_OBJECT
public:
    L10nMoneyNodeFactory();

    Node *getNode(const QString &tagContent, Parser *p) const override;
};

class L10nMoneyVarNodeFactory : public AbstractNodeFactory
{
    Q_OBJECT
public:
    L10nMoneyVarNodeFactory();

    Node *getNode(const QString &tagContent, Parser *p) const override;
};

class L10nMoneyNode : public Node
{
    Q_OBJECT
public:
    L10nMoneyNode(const FilterExpression &value, const FilterExpression &currency, QObject *parent = {});
    void render(OutputStream *stream, Context *c) const override;

private:
    FilterExpression m_value;
    FilterExpression m_currency;
};

class L10nMoneyVarNode : public Node
{
    Q_OBJECT
public:
    L10nMoneyVarNode(const FilterExpression &value, const FilterExpression &currency, const QString &resultName, QObject *parent = {});
    void render(OutputStream *stream, Context *c) const override;

private:
    FilterExpression m_value;
    FilterExpression m_currency;
    QString m_resultName;
};

#endif
