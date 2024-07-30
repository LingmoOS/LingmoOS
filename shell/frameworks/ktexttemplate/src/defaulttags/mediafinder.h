/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2009, 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#ifndef MEDIAFINDER_H
#define MEDIAFINDER_H

#include "node.h"

using namespace KTextTemplate;

class MediaFinderNodeFactory : public AbstractNodeFactory
{
public:
    MediaFinderNodeFactory();

    Node *getNode(const QString &tagContent, Parser *p) const override;
};

class MediaFinderNode : public Node
{
    Q_OBJECT
public:
    explicit MediaFinderNode(const QList<FilterExpression> &mediaExpressionList, QObject *parent = {});

    void render(OutputStream *stream, Context *c) const override;

private:
    QList<FilterExpression> m_mediaExpressionList;
};

#endif
