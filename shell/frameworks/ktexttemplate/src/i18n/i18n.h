/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.0-or-later

*/

#ifndef I18NNODE_H
#define I18NNODE_H

#include "node.h"

namespace KTextTemplate
{
class Parser;
}

using namespace KTextTemplate;

class I18nNodeFactory : public AbstractNodeFactory
{
    Q_OBJECT
public:
    I18nNodeFactory();

    Node *getNode(const QString &tagContent, Parser *p) const override;
};

class I18nVarNodeFactory : public AbstractNodeFactory
{
    Q_OBJECT
public:
    I18nVarNodeFactory();

    Node *getNode(const QString &tagContent, Parser *p) const override;
};

class I18nNode : public Node
{
    Q_OBJECT
public:
    I18nNode(const QString &sourceText, const QList<FilterExpression> &feList, QObject *parent = {});
    void render(OutputStream *stream, Context *c) const override;

private:
    QString m_sourceText;
    QList<FilterExpression> m_filterExpressionList;
};

class I18nVarNode : public Node
{
    Q_OBJECT
public:
    I18nVarNode(const QString &sourceText, const QList<FilterExpression> &feList, const QString &resultName, QObject *parent = {});
    void render(OutputStream *stream, Context *c) const override;

private:
    QString m_sourceText;
    QList<FilterExpression> m_filterExpressionList;
    QString m_resultName;
};

#endif
