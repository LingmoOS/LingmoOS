/*
    SPDX-FileCopyrightText: 2007 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATEARGUMENTHINTMODEL_H
#define KATEARGUMENTHINTMODEL_H

#include "katecompletionmodel.h"
#include <QAbstractListModel>

class KateCompletionWidget;

class KateArgumentHintModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit KateArgumentHintModel(KateCompletionModel *parent);

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    int rowCount(const QModelIndex &parent = {}) const override;

    void emitDataChanged(const QModelIndex &start, const QModelIndex &end);

    // Returns the index in the source-model for an index within this model
    QModelIndex mapToSource(const QModelIndex &proxyIndex) const;

    void buildRows();
    void clear();

public Q_SLOTS:
    void parentModelReset();

Q_SIGNALS:
    void contentStateChanged(bool hasContent);

private:
    KateCompletionModel::Group *group() const;

    std::vector<int> m_rows; // Maps rows to either a positive row-number in the source group, or to a negative number which indicates a label
    KateCompletionModel *m_parent;
};

#endif
