/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.0-or-later

*/

#ifndef I18NCNODE_H
#define I18NCNODE_H

#include "node.h"

namespace KTextTemplate
{
class Parser;
}

using namespace KTextTemplate;

class I18ncNodeFactory : public AbstractNodeFactory
{
    Q_OBJECT
public:
    I18ncNodeFactory();

    Node *getNode(const QString &tagContent, Parser *p) const override;
};

class I18ncVarNodeFactory : public AbstractNodeFactory
{
    Q_OBJECT
public:
    I18ncVarNodeFactory();

    Node *getNode(const QString &tagContent, Parser *p) const override;
};

class I18ncNode : public Node
{
    Q_OBJECT
public:
    I18ncNode(const QString &sourceText, const QString &context, const QList<FilterExpression> &feList, QObject *parent = {});
    void render(OutputStream *stream, Context *c) const override;

private:
    QString m_sourceText;
    QString m_context;
    QList<FilterExpression> m_filterExpressionList;
};

class I18ncVarNode : public Node
{
    Q_OBJECT
public:
    I18ncVarNode(const QString &sourceText, const QString &context, const QList<FilterExpression> &feList, const QString &resultName, QObject *parent = {});
    void render(OutputStream *stream, Context *c) const override;

private:
    QString m_sourceText;
    QString m_context;
    QList<FilterExpression> m_filterExpressionList;
    QString m_resultName;
};

#endif
