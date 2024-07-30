/*
    SPDX-FileCopyrightText: 2009 Stephen Kelly <steveire@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef INDEXFINDER_H
#define INDEXFINDER_H

#include <QModelIndex>

class IndexFinder
{
public:
    IndexFinder(QList<int> rows = QList<int>())
        : m_rows(rows)
        , m_model(nullptr)
    {
    }

    IndexFinder(const QAbstractItemModel *model, QList<int> rows = QList<int>())
        : m_rows(rows)
        , m_model(model)
    {
        Q_ASSERT(model);
    }

    QModelIndex getIndex() const
    {
        if (!m_model) {
            return QModelIndex();
        }
        static const int col = 0;
        QModelIndex parent = QModelIndex();
        QListIterator<int> i(m_rows);
        while (i.hasNext()) {
            parent = m_model->index(i.next(), col, parent);
            Q_ASSERT(parent.isValid());
        }
        return parent;
    }

    static IndexFinder indexToIndexFinder(const QModelIndex &_idx)
    {
        if (!_idx.isValid()) {
            return IndexFinder();
        }

        QList<int> list;
        QModelIndex idx = _idx;
        while (idx.isValid()) {
            list.prepend(idx.row());
            idx = idx.parent();
        }
        return IndexFinder(_idx.model(), list);
    }

    bool operator==(const IndexFinder &other) const
    {
        return (m_rows == other.m_rows && m_model == other.m_model);
    }

    QList<int> rows() const
    {
        return m_rows;
    }
    void appendRow(int row)
    {
        m_rows.append(row);
    }
    void setRows(const QList<int> &rows)
    {
        m_rows = rows;
    }
    void setModel(QAbstractItemModel *model)
    {
        m_model = model;
    }

private:
    QList<int> m_rows;
    const QAbstractItemModel *m_model;
};

Q_DECLARE_METATYPE(IndexFinder)

#endif
