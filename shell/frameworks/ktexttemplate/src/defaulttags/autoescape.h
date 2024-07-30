/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2009, 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#ifndef AUTOESCAPENODE_H
#define AUTOESCAPENODE_H

#include "node.h"

using namespace KTextTemplate;

class AutoescapeNodeFactory : public AbstractNodeFactory
{
    Q_OBJECT
public:
    AutoescapeNodeFactory();

    Node *getNode(const QString &tagContent, Parser *p) const override;
};

class AutoescapeNode : public Node
{
    Q_OBJECT
public:
    enum State { On, Off };

    explicit AutoescapeNode(int state, QObject *parent = {});

    void setList(const NodeList &list);

    void render(OutputStream *stream, Context *c) const override;

private:
    NodeList m_list;
    int m_state;
};

#endif
