/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#pragma once

#include <QSortFilterProxyModel>
#include <qqmlregistration.h>

#include <KUser>

class ProcessSortFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(QString filterString READ filterString WRITE setFilterString NOTIFY filterStringChanged)
    Q_PROPERTY(ViewMode viewMode READ viewMode WRITE setViewMode NOTIFY viewModeChanged)
    Q_PROPERTY(QStringList hiddenAttributes READ hiddenAttributes WRITE setHiddenAttributes NOTIFY hiddenAttributesChanged)
    Q_PROPERTY(QVariantList filterPids READ filterPids WRITE setFilterPids NOTIFY filterPidsChanged)

    Q_PROPERTY(int sortColumn READ sortColumn NOTIFY sorted)
    Q_PROPERTY(int sortOrder READ sortOrder NOTIFY sorted)

    QML_ELEMENT

public:
    enum ViewMode {
        ViewOwn,
        ViewUser,
        ViewSystem,
        ViewAll,
    };
    Q_ENUM(ViewMode)

    explicit ProcessSortFilterModel(QObject *parent = nullptr);

    void setSourceModel(QAbstractItemModel *newSourceModel) override;
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
    bool filterAcceptsColumn(int sourceColumn, const QModelIndex &sourceParent) const override;

    QString filterString() const;
    void setFilterString(const QString &newFilterString);
    Q_SIGNAL void filterStringChanged();

    ViewMode viewMode() const;
    void setViewMode(ViewMode newViewMode);
    Q_SIGNAL void viewModeChanged();

    QStringList hiddenAttributes() const;
    void setHiddenAttributes(const QStringList &newHiddenAttributes);
    Q_SIGNAL void hiddenAttributesChanged();

    QVariantList filterPids() const;
    void setFilterPids(const QVariantList &newFilterPids);
    Q_SIGNAL void filterPidsChanged();

    Q_INVOKABLE void sort(int column, Qt::SortOrder order) override;
    Q_SIGNAL void sorted();

private:
    void findColumns();

    QString m_filterString;
    ViewMode m_viewMode = ViewOwn;
    QStringList m_hiddenAttributes;
    QVariantList m_filterPids;

    int m_uidColumn = -1;
    int m_pidColumn = -1;
    KUser m_currentUser;
};
