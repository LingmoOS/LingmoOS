/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2009, 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#ifndef INCLUDENODE_H
#define INCLUDENODE_H

#include "node.h"

namespace KTextTemplate
{
class Parser;
}

using namespace KTextTemplate;

class IncludeNodeFactory : public AbstractNodeFactory
{
    Q_OBJECT
public:
    IncludeNodeFactory();

    Node *getNode(const QString &tagContent, Parser *p) const override;
};

class IncludeNode : public Node
{
    Q_OBJECT
public:
    explicit IncludeNode(const FilterExpression &fe, QObject *parent = {});
    void render(OutputStream *stream, Context *c) const override;

private:
    FilterExpression m_filterExpression;
};

class ConstantIncludeNode : public Node
{
    Q_OBJECT
public:
    explicit ConstantIncludeNode(const QString &filename, QObject *parent = {});
    void render(OutputStream *stream, Context *c) const override;

private:
    QString m_name;
};

#endif
