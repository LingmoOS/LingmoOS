/*
    SPDX-FileCopyrightText: 2009 Stephen Kelly <steveire@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef MODELSPY_H
#define MODELSPY_H

#include <QModelIndex>
#include <QObject>
#include <QVariantList>

#include "persistentchangelist.h"
#include <QItemSelectionRange>

#include "proxymodeltestsuite_export.h"

enum SignalType {
    NoSignal,
    RowsAboutToBeInserted,
    RowsInserted,
    RowsAboutToBeRemoved,
    RowsRemoved,
    RowsAboutToBeMoved,
    RowsMoved,
    DataChanged,
    LayoutAboutToBeChanged,
    LayoutChanged,
    ModelAboutToBeReset,
    ModelReset,
};

class PROXYMODELTESTSUITE_EXPORT ModelSpy : public QObject, public QList<QVariantList>
{
    Q_OBJECT
public:
    ModelSpy(QObject *parent);

    void setModel(QAbstractItemModel *model);
    bool useLazyPersistence() const
    {
        return m_lazyPersist;
    }
    void setLazyPersistence(bool lazy)
    {
        m_lazyPersist = lazy;
    }

    void preTestPersistIndexes(const PersistentChangeList &changeList);
    QModelIndexList getUnchangedIndexes() const
    {
        return m_unchangedIndexes;
    }
    QList<QPersistentModelIndex> getUnchangedPersistentIndexes() const
    {
        return m_unchangedPersistentIndexes;
    }
    PersistentChangeList getChangeList() const
    {
        return m_changeList;
    }

    void startSpying();
    void stopSpying();
    bool isSpying()
    {
        return m_isSpying;
    }

    void clearTestData();

protected Q_SLOTS:
    void rowsAboutToBeInserted(const QModelIndex &parent, int start, int end);
    void rowsInserted(const QModelIndex &parent, int start, int end);
    void rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end);
    void rowsRemoved(const QModelIndex &parent, int start, int end);
    void rowsAboutToBeMoved(const QModelIndex &srcParent, int start, int end, const QModelIndex &destParent, int destStart);
    void rowsMoved(const QModelIndex &srcParent, int start, int end, const QModelIndex &destParent, int destStart);

    void layoutAboutToBeChanged();
    void layoutChanged();

    void modelAboutToBeReset();
    void modelReset();

    void modelDestroyed();

    void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);

private:
    void doPersist();
    QModelIndexList getUnchangedIndexes(const QModelIndex &parent, const QList<QItemSelectionRange> &ignoredRanges);
    QModelIndexList getDescendantIndexes(const QModelIndex &index);
    QList<QPersistentModelIndex> toPersistent(const QModelIndexList &list);

private:
    QAbstractItemModel *m_model;
    bool m_isSpying;
    bool m_lazyPersist;
    PersistentChangeList m_changeList;
    QModelIndexList m_unchangedIndexes;
    QList<QPersistentModelIndex> m_unchangedPersistentIndexes;
};

PROXYMODELTESTSUITE_EXPORT uint qHash(const QVariant &var);

PROXYMODELTESTSUITE_EXPORT QDebug operator<<(QDebug d, ModelSpy *modelSpy);

#endif
