/*
    SPDX-FileCopyrightText: 2009 Stephen Kelly <steveire@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "modelspy.h"

#include <QDebug>

ModelSpy::ModelSpy(QObject *parent)
    : QObject(parent)
    , QList<QVariantList>()
    , m_model(nullptr)
    , m_isSpying(false)
    , m_lazyPersist(false)
{
}

void ModelSpy::setModel(QAbstractItemModel *model)
{
    stopSpying();
    m_model = model;
}

void ModelSpy::clearTestData()
{
    m_changeList.clear();
    m_unchangedIndexes.clear();
    m_unchangedPersistentIndexes.clear();
}

void ModelSpy::startSpying()
{
    // If a signal is connected to a slot multiple times, the slot gets called multiple times.
    // As we're doing start and stop spying all the time, we disconnect here first to make sure.
    stopSpying();

    m_isSpying = true;

    connect(m_model, &QAbstractItemModel::rowsAboutToBeInserted, this, &ModelSpy::rowsAboutToBeInserted);
    connect(m_model, &QAbstractItemModel::rowsInserted, this, &ModelSpy::rowsInserted);
    connect(m_model, &QAbstractItemModel::rowsAboutToBeRemoved, this, &ModelSpy::rowsAboutToBeRemoved);
    connect(m_model, &QAbstractItemModel::rowsRemoved, this, &ModelSpy::rowsRemoved);
    connect(m_model, &QAbstractItemModel::layoutAboutToBeChanged, this, &ModelSpy::layoutAboutToBeChanged);
    connect(m_model, &QAbstractItemModel::layoutChanged, this, &ModelSpy::layoutChanged);
    connect(m_model, &QAbstractItemModel::modelAboutToBeReset, this, &ModelSpy::modelAboutToBeReset);
    connect(m_model, &QAbstractItemModel::modelReset, this, &ModelSpy::modelReset);
    connect(m_model, &QAbstractItemModel::rowsAboutToBeMoved, this, &ModelSpy::rowsAboutToBeMoved);
    connect(m_model, &QAbstractItemModel::rowsMoved, this, &ModelSpy::rowsMoved);
    connect(m_model, &QAbstractItemModel::dataChanged, this, &ModelSpy::dataChanged);
    connect(m_model, &QObject::destroyed, this, &ModelSpy::modelDestroyed);
}

void ModelSpy::stopSpying()
{
    m_isSpying = false;
    if (!m_model) {
        return;
    }

    disconnect(m_model, &QAbstractItemModel::rowsAboutToBeInserted, this, &ModelSpy::rowsAboutToBeInserted);
    disconnect(m_model, &QAbstractItemModel::rowsInserted, this, &ModelSpy::rowsInserted);
    disconnect(m_model, &QAbstractItemModel::rowsAboutToBeRemoved, this, &ModelSpy::rowsAboutToBeRemoved);
    disconnect(m_model, &QAbstractItemModel::rowsRemoved, this, &ModelSpy::rowsRemoved);
    disconnect(m_model, &QAbstractItemModel::layoutAboutToBeChanged, this, &ModelSpy::layoutAboutToBeChanged);
    disconnect(m_model, &QAbstractItemModel::layoutChanged, this, &ModelSpy::layoutChanged);
    disconnect(m_model, &QAbstractItemModel::modelAboutToBeReset, this, &ModelSpy::modelAboutToBeReset);
    disconnect(m_model, &QAbstractItemModel::modelReset, this, &ModelSpy::modelReset);
    disconnect(m_model, &QAbstractItemModel::rowsAboutToBeMoved, this, &ModelSpy::rowsAboutToBeMoved);
    disconnect(m_model, &QAbstractItemModel::rowsMoved, this, &ModelSpy::rowsMoved);
    disconnect(m_model, &QAbstractItemModel::dataChanged, this, &ModelSpy::dataChanged);
    disconnect(m_model, &QObject::destroyed, this, &ModelSpy::modelDestroyed);
}

void ModelSpy::rowsAboutToBeInserted(const QModelIndex &parent, int start, int end)
{
    append(QVariantList() << RowsAboutToBeInserted << QVariant::fromValue(parent) << start << end);

    if (m_lazyPersist) {
        doPersist();
    }
}

void ModelSpy::rowsInserted(const QModelIndex &parent, int start, int end)
{
    append(QVariantList() << RowsInserted << QVariant::fromValue(parent) << start << end);
}

void ModelSpy::rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
{
    append(QVariantList() << RowsAboutToBeRemoved << QVariant::fromValue(parent) << start << end);

    if (m_lazyPersist) {
        doPersist();
    }
}

void ModelSpy::rowsRemoved(const QModelIndex &parent, int start, int end)
{
    append(QVariantList() << RowsRemoved << QVariant::fromValue(parent) << start << end);
}

void ModelSpy::layoutAboutToBeChanged()
{
    append(QVariantList() << LayoutAboutToBeChanged);

    if (m_lazyPersist) {
        doPersist();
    }
}

void ModelSpy::layoutChanged()
{
    append(QVariantList() << LayoutChanged);
}

void ModelSpy::modelAboutToBeReset()
{
    append(QVariantList() << ModelAboutToBeReset);

    // This is called in setSourceModel for example, which is not when we want to persist.
    if (m_lazyPersist && m_model->hasChildren()) {
        doPersist();
    }
}

void ModelSpy::modelReset()
{
    append(QVariantList() << ModelReset);
}

void ModelSpy::modelDestroyed()
{
    stopSpying();
    m_model = nullptr;
}

void ModelSpy::rowsAboutToBeMoved(const QModelIndex &srcParent, int start, int end, const QModelIndex &destParent, int destStart)
{
    append(QVariantList() << RowsAboutToBeMoved << QVariant::fromValue(srcParent) << start << end << QVariant::fromValue(destParent) << destStart);

    // Don't do a lazy persist here. That will be done on the layoutAboutToBeChanged signal.
    //   if (m_lazyPersist)
    //     doPersist();
}

void ModelSpy::rowsMoved(const QModelIndex &srcParent, int start, int end, const QModelIndex &destParent, int destStart)
{
    append(QVariantList() << RowsMoved << QVariant::fromValue(srcParent) << start << end << QVariant::fromValue(destParent) << destStart);
}

void ModelSpy::dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    append(QVariantList() << DataChanged << QVariant::fromValue(topLeft) << QVariant::fromValue(bottomRight));
}

QModelIndexList ModelSpy::getDescendantIndexes(const QModelIndex &parent)
{
    QModelIndexList list;
    const int column = 0;
    for (int row = 0; row < m_model->rowCount(parent); ++row) {
        QModelIndex idx = m_model->index(row, column, parent);
        list << idx;
        list << getDescendantIndexes(idx);
    }
    return list;
}

QList<QPersistentModelIndex> ModelSpy::toPersistent(const QModelIndexList &list)
{
    QList<QPersistentModelIndex> persistentList;
    for (const QModelIndex &idx : list) {
        persistentList << QPersistentModelIndex(idx);
    }
    return persistentList;
}

QModelIndexList ModelSpy::getUnchangedIndexes(const QModelIndex &parent, const QList<QItemSelectionRange> &ignoredRanges)
{
    QModelIndexList list;
    int rowCount = m_model->rowCount(parent);
    for (int row = 0; row < rowCount;) {
        int column = 0;
        QModelIndex idx = m_model->index(row, column, parent);
        Q_ASSERT(idx.isValid());
        bool found = false;
        for (const QItemSelectionRange &range : ignoredRanges) {
            if (range.topLeft().parent() == parent && range.topLeft().row() == idx.row()) {
                row = range.bottomRight().row() + 1;
                found = true;
                break;
            }
        }
        if (!found) {
            for (column = 0; column < m_model->columnCount(); ++column) {
                list << m_model->index(row, column, parent);
            }
            list << getUnchangedIndexes(idx, ignoredRanges);
            ++row;
        }
    }
    return list;
}

void ModelSpy::preTestPersistIndexes(const PersistentChangeList &changeList)
{
    m_changeList = changeList;
    if (!m_lazyPersist) {
        doPersist();
    }
}

void ModelSpy::doPersist()
{
    Q_ASSERT(m_unchangedIndexes.isEmpty());
    Q_ASSERT(m_unchangedPersistentIndexes.isEmpty());

    const int columnCount = m_model->columnCount();
    QMutableListIterator<PersistentIndexChange> it(m_changeList);

    // The indexes are defined by the test are described with IndexFinder before anything in the model exists.
    // Now that the indexes should exist, resolve them in the change objects.
    QList<QItemSelectionRange> changedRanges;

    while (it.hasNext()) {
        PersistentIndexChange change = it.next();
        change.parentFinder.setModel(m_model);
        QModelIndex parent = change.parentFinder.getIndex();

        Q_ASSERT(change.startRow >= 0);
        Q_ASSERT(change.startRow <= change.endRow);

        if (change.endRow >= m_model->rowCount(parent)) {
            qDebug() << m_model << parent << change.startRow << change.endRow << parent.data() << m_model->rowCount(parent);
        }

        Q_ASSERT(change.endRow < m_model->rowCount(parent));

        QModelIndex topLeft = m_model->index(change.startRow, 0, parent);
        QModelIndex bottomRight = m_model->index(change.endRow, columnCount - 1, parent);

        // We store the changed ranges so that we know which ranges should not be changed
        changedRanges << QItemSelectionRange(topLeft, bottomRight);

        // Store the initial state of the indexes in the model which we expect to change.
        for (int row = change.startRow; row <= change.endRow; ++row) {
            for (int column = 0; column < columnCount; ++column) {
                QModelIndex idx = m_model->index(row, column, parent);
                Q_ASSERT(idx.isValid());
                change.indexes << idx;
                change.persistentIndexes << QPersistentModelIndex(idx);
            }

            // Also store the descendants of changed indexes so that we can verify the effect on them
            QModelIndex idx = m_model->index(row, 0, parent);
            QModelIndexList descs = getDescendantIndexes(idx);
            change.descendantIndexes << descs;
            change.persistentDescendantIndexes << toPersistent(descs);
        }
        it.setValue(change);
    }
    // Any indexes outside of the ranges we expect to be changed are stored
    // so that we can later verify that they remain unchanged.
    m_unchangedIndexes = getUnchangedIndexes(QModelIndex(), changedRanges);
    m_unchangedPersistentIndexes = toPersistent(m_unchangedIndexes);
}

static const char *const signaltypes[] = {"NoSignal",
                                          "RowsAboutToBeInserted",
                                          "RowsInserted",
                                          "RowsAboutToBeRemoved",
                                          "RowsRemoved",
                                          "RowsAboutToBeMoved",
                                          "RowsMoved",
                                          "DataChanged",
                                          "LayoutAboutToBeChanged",
                                          "LayoutChanged",
                                          "ModelAboutToBeReset",
                                          "ModelReset"};

QDebug operator<<(QDebug d, ModelSpy *modelSpy)
{
    d << "ModelSpy(";
    for (const QVariantList &list : std::as_const(*modelSpy)) {
        d << "SIGNAL(";
        int sigType = list.first().toInt();
        d << signaltypes[sigType];
        if (list.size() > 1) {
            d << ", " << list.mid(1);
        }
        d << ")";
    }
    d << ")";
    return d;
}

#include "moc_modelspy.cpp"
