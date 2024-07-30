/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.0-or-later

*/

#ifndef I18NPNODE_H
#define I18NPNODE_H

#include "node.h"

namespace KTextTemplate
{
class Parser;
}

using namespace KTextTemplate;

class I18npNodeFactory : public AbstractNodeFactory
{
    Q_OBJECT
public:
    I18npNodeFactory();

    Node *getNode(const QString &tagContent, Parser *p) const override;
};

class I18npVarNodeFactory : public AbstractNodeFactory
{
    Q_OBJECT
public:
    I18npVarNodeFactory();

    Node *getNode(const QString &tagContent, Parser *p) const override;
};

class I18npNode : public Node
{
    Q_OBJECT
public:
    I18npNode(const QString &sourceText, const QString &pluralText, const QList<FilterExpression> &feList, QObject *parent = {});
    void render(OutputStream *stream, Context *c) const override;

private:
    QString m_sourceText;
    QString m_pluralText;
    QList<FilterExpression> m_filterExpressionList;
};

class I18npVarNode : public Node
{
    Q_OBJECT
public:
    I18npVarNode(const QString &sourceText, const QString &pluralText, const QList<FilterExpression> &feList, const QString &resultName, QObject *parent = {});
    void render(OutputStream *stream, Context *c) const override;

private:
    QString m_sourceText;
    QString m_pluralText;
    QList<FilterExpression> m_filterExpressionList;
    QString m_resultName;
};

#endif
