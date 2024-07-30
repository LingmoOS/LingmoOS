/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2009, 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#ifndef IFNODE_H
#define IFNODE_H

#include "node.h"

#include <QSharedPointer>

using namespace KTextTemplate;

class IfNodeFactory : public AbstractNodeFactory
{
    Q_OBJECT
public:
    IfNodeFactory();

    Node *getNode(const QString &tagContent, Parser *p) const override;
};

class IfToken;

class IfNode : public Node
{
    Q_OBJECT
public:
    explicit IfNode(QObject *parent = {});

    void setNodelistConditions(const QList<std::pair<QSharedPointer<IfToken>, NodeList>> &conditionNodelists);

    void render(OutputStream *stream, Context *c) const override;

private:
    QList<std::pair<QSharedPointer<IfToken>, NodeList>> mConditionNodelists;
};

#endif
