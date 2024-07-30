/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2009, 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#ifndef SCRIPTABLE_TEMPLATE_H
#define SCRIPTABLE_TEMPLATE_H

#include <QObject>

#include "scriptablecontext.h"
#include "template.h"

namespace KTextTemplate
{
class Node;
}

using namespace KTextTemplate;

class ScriptableTemplate : public QObject
{
    Q_OBJECT
public:
    explicit ScriptableTemplate(Template t, QObject *parent = {});

public Q_SLOTS:
    QString render(ScriptableContext *c) const;

    QList<QObject *> nodeList() const;

    void setNodeList(const QList<QObject *> &list);

private:
    Template m_template;
};

#endif
