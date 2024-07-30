/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.0-or-later

*/

#ifndef I18NCPNODE_H
#define I18NCPNODE_H

#include "node.h"

namespace KTextTemplate
{
class Parser;
}

using namespace KTextTemplate;

class I18ncpNodeFactory : public AbstractNodeFactory
{
    Q_OBJECT
public:
    I18ncpNodeFactory();

    Node *getNode(const QString &tagContent, Parser *p) const override;
};

class I18ncpVarNodeFactory : public AbstractNodeFactory
{
    Q_OBJECT
public:
    I18ncpVarNodeFactory();

    Node *getNode(const QString &tagContent, Parser *p) const override;
};

class I18ncpNode : public Node
{
    Q_OBJECT
public:
    I18ncpNode(const QString &contextText, const QString &sourceText, const QString &pluralTex, const QList<FilterExpression> &feList, QObject *parent = {});
    void render(OutputStream *stream, Context *c) const override;

private:
    QString m_contextText;
    QString m_sourceText;
    QString m_pluralText;
    QList<FilterExpression> m_filterExpressionList;
};

class I18ncpVarNode : public Node
{
    Q_OBJECT
public:
    I18ncpVarNode(const QString &contextText,
                  const QString &sourceText,
                  const QString &pluralText,
                  const QList<FilterExpression> &feList,
                  const QString &resultName,
                  QObject *parent = {});
    void render(OutputStream *stream, Context *c) const override;

private:
    QString m_contextText;
    QString m_sourceText;
    QString m_pluralText;
    QList<FilterExpression> m_filterExpressionList;
    QString m_resultName;
};

#endif
