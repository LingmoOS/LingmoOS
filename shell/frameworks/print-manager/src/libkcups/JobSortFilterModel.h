/*
    SPDX-FileCopyrightText: 2012-2013 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef JOB_SORT_FILTER_MODEL_H
#define JOB_SORT_FILTER_MODEL_H

#include <QSortFilterProxyModel>
#include <kcupslib_export.h>
#include <qqmlregistration.h>

class KCUPSLIB_EXPORT JobSortFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QString filteredPrinters READ filteredPrinters WRITE setFilteredPrinters NOTIFY filteredPrintersChanged)
    Q_PROPERTY(QAbstractItemModel *sourceModel READ sourceModel WRITE setModel NOTIFY sourceModelChanged)
    Q_PROPERTY(int activeCount READ activeCount NOTIFY activeCountChanged)
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    explicit JobSortFilterModel(QObject *parent = nullptr);

    void setModel(QAbstractItemModel *model);
    void setFilteredPrinters(const QString &printers);
    QString filteredPrinters() const;
    int activeCount() const;
    int count() const;
signals:
    void activeCountChanged();
    void countChanged();
    void sourceModelChanged(QObject *);
    void filteredPrintersChanged();

private:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;

    int weightForState(int state) const;

    QStringList m_filteredPrinters;
};

#endif // JOB_SORT_FILTER_MODEL_H
