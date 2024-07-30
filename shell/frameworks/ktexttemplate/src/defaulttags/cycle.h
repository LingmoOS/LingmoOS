/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2009, 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#ifndef CYCLENODE_H
#define CYCLENODE_H

#include "node.h"

using namespace KTextTemplate;

class CycleNodeFactory : public AbstractNodeFactory
{
    Q_OBJECT
public:
    CycleNodeFactory();

    Node *getNode(const QString &tagContent, Parser *p) const override;
};

/**
Cycles endlessly over elements.
*/
template<typename T>
class RingIterator
{
public:
    RingIterator()
    {
    }

    RingIterator(const QList<T> &list)
        : m_begin(list.constBegin())
        , m_it(list.constBegin())
        , m_end(list.constEnd())
    {
        Q_ASSERT(!list.isEmpty());
    }

    /**
      Returns the next element in the list, or the first element if already
      at the last element.
    */
    T next()
    {
        Q_ASSERT(m_it != m_end);

        const T t = *m_it++;
        if (m_it == m_end)
            m_it = m_begin;
        return t;
    }

private:
    typename QList<T>::const_iterator m_begin;
    typename QList<T>::const_iterator m_it;
    typename QList<T>::const_iterator m_end;
};

typedef RingIterator<FilterExpression> FilterExpressionRotator;

Q_DECLARE_METATYPE(FilterExpressionRotator)

class CycleNode : public Node
{
    Q_OBJECT
public:
    CycleNode(const QList<FilterExpression> &list, const QString &name, QObject *parent = {});

    void render(OutputStream *stream, Context *c) const override;

private:
    const QList<FilterExpression> m_list;
    FilterExpressionRotator m_variableIterator;
    const QString m_name;
};

#endif
