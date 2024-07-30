/*
 * SPDX-FileCopyrightText: 2015 Dominik Haumann <dhaumann@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#ifndef PLASMA_QUOTA_LIST_MODEL_H
#define PLASMA_QUOTA_LIST_MODEL_H

#include <QAbstractListModel>
#include <QList>

#include "QuotaItem.h"

/**
 * Data model holding disk quota items.
 */
class QuotaListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit QuotaListModel(QObject *parent = nullptr);

public: // QAbstractListModel overrides
    /**
     * List of available roles for the QML ListView.
     */
    QHash<int, QByteArray> roleNames() const override;

    /**
     * Returns the data for @p index and given @p role.
     */
    QVariant data(const QModelIndex &index, int role) const override;

    /**
     * Returns the number of items for the toplevel model index, otherwise 0.
     */
    int rowCount(const QModelIndex &index) const override;

    /**
     * Changes the data for @p index to @p variant.
     */
    bool setData(const QModelIndex &index, const QVariant &variant, int role = Qt::EditRole) override;

    /**
     * Inserts @p count rows at position @p row.
     */
    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

    /**
     * Removes @p count rows at position @p row.
     */
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

public: // additional helper functions
    /**
     * Merges @p items into the existing quota item list. Old items that are
     * not available in @p items anymore are deleted.
     */
    void updateItems(const QList<QuotaItem> &items);

    /**
     * Clears all items in the model.
     */
    void clear();

private:
    QList<QuotaItem> m_items;
};

#endif // PLASMA_QUOTA_LIST_MODEL_H
