/*
    SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KPLUGINPROXYMODEL_H
#define KPLUGINPROXYMODEL_H

#include "kcmutilscore_export.h"
#include "kpluginmodel.h"

#include <KCategorizedSortFilterProxyModel>

class Q_DECL_HIDDEN KPluginProxyModel : public KCategorizedSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(QString query READ query WRITE setQuery NOTIFY queryChanged)
    Q_PROPERTY(QAbstractListModel *model WRITE setModel)
public:
    explicit KPluginProxyModel(QObject *parent = nullptr);
    ~KPluginProxyModel() override;

    QString query() const;
    void setQuery(const QString &query);
    void setModel(QAbstractListModel *model)
    {
        setSourceModel(model);
        m_model = qobject_cast<KPluginModel *>(model);
        Q_ASSERT(m_model);
    }

Q_SIGNALS:
    void queryChanged();

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
    bool subSortLessThan(const QModelIndex &left, const QModelIndex &right) const override;
    int compareCategories(const QModelIndex &left, const QModelIndex &right) const override;

private:
    QString m_query;
    KPluginModel *m_model;
};

#endif
