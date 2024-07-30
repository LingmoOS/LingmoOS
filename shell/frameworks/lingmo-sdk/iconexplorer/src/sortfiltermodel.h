/*
    SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QSortFilterProxyModel>

namespace CuttleFish
{
class SortFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(QString filter READ filter WRITE setFilter NOTIFY filterChanged)
    Q_PROPERTY(QString category READ category WRITE setCategory NOTIFY categoryChanged)
    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentIndexChanged)
public:
    explicit SortFilterModel(QObject *parent = nullptr);

    void setCategory(const QString &category);
    QString category() const;

    void setFilter(const QString &filter);
    QString filter() const;

    void setCurrentIndex(int index);
    int currentIndex();

Q_SIGNALS:
    void filterChanged();
    void categoryChanged();
    void currentIndexChanged();

private:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

    QString m_category;
    QString m_filter;
    QModelIndex m_currentSourceIndex;
};

}
