/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2009, 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#ifndef NOWNODE_H
#define NOWNODE_H

#include "node.h"

using namespace KTextTemplate;

class NowNodeFactory : public AbstractNodeFactory
{
    Q_OBJECT
public:
    NowNodeFactory();

    Node *getNode(const QString &tagContent, Parser *p) const override;
};

class NowNode : public Node
{
    Q_OBJECT
public:
    explicit NowNode(const QString &formatString, QObject *parent = {});

    void render(OutputStream *stream, Context *c) const override;

private:
    QString m_formatString;
};

#endif
