// SPDX-FileCopyrightText: 2010 by Marco Martin <mart@kde.org>
// SPDX-FileCopyrightText: 2019 by David Edmundson <davidedmundson@kde.org>
//
// SPDX-License-Identifier: LGPL-2.0-or-later
//
// Source code extracted from KItemModels with minimized changes.
// This should be removed once the Qt 6 based KItemModels package is
// available from deepin's package repo.

#ifndef KSORTFILTERPROXYMODEL_H
#define KSORTFILTERPROXYMODEL_H

#include <QAbstractItemModel>
#include <QJSValue>
#include <QList>
#include <QQmlParserStatus>
#include <QSortFilterProxyModel>
#include <QQmlEngine>

/**
 * @class SortFilterModel
 * @short Filter and sort an existing QAbstractItemModel
 *
 * @since 5.67
 */
class KSortFilterProxyModel : public QSortFilterProxyModel, public QQmlParserStatus
{
    Q_OBJECT
    QML_NAMED_ELEMENT(SortFilterProxyModel)
    Q_INTERFACES(QQmlParserStatus)
    /**
     * The source model of this sorting proxy model.
     */
    Q_PROPERTY(QAbstractItemModel *sourceModel READ sourceModel WRITE setModel NOTIFY sourceModelChanged)

    /**
     * The string for the filter, only rows with their filterRole matching filterString will be displayed
     */
    Q_PROPERTY(QString filterString READ filterString WRITE setFilterString NOTIFY filterStringChanged)
    /**
     * A JavaScript callable that can be used to perform advanced filters on a given row.
     * The callback is passed the source row, and source parent for a given row as arguments
     *
     * The callable's return value is evaluated as boolean to determine
     * whether the row is accepted (true) or filtered out (false). It overrides the default implementation
     * that uses filterRegExp or filterString; while filterCallback is set those two properties are
     * ignored. Attempts to write a non-callable to this property are silently ignored, but you can set
     * it to null.
     *
     * @code
     * filterRowCallback: function(source_row, source_parent) {
     *   return sourceModel.data(sourceModel.index(source_row, 0, source_parent), Qt.DisplayRole) == "...";
     * };
     * @endcode
     */
    Q_PROPERTY(QJSValue filterRowCallback READ filterRowCallback WRITE setFilterRowCallback NOTIFY filterRowCallbackChanged)

    /**
     * A JavaScript callable that can be used to perform advanced filters on a given column.
     * The callback is passed the source column, and source parent for a given column as arguments.
     *
     * @see filterRowCallback
     */
    Q_PROPERTY(QJSValue filterColumnCallback READ filterColumnCallback WRITE setFilterColumnCallback NOTIFY filterColumnCallbackChanged)

    /**
     * The role of the sourceModel on which the filter will be applied.
     * This can either be the numerical role value or the role name as a string.
     */
    Q_PROPERTY(QString filterRoleName READ filterRoleName WRITE setFilterRoleName NOTIFY filterRoleNameChanged)

    /**
     * The role of the sourceModel that will be used for sorting. if empty the order will be left unaltered
     * This can either be the numerical role value or the role name as a string.
     */
    Q_PROPERTY(QString sortRoleName READ sortRoleName WRITE setSortRoleName NOTIFY sortRoleNameChanged)

    /**
     * One of Qt.AscendingOrder or Qt.DescendingOrder
     */
    Q_PROPERTY(Qt::SortOrder sortOrder READ sortOrder WRITE setSortOrder NOTIFY sortOrderChanged)

    /**
     * Specify which column should be used for sorting
     * The default value is -1.
     * If \a sortRole is set, the default value is 0.
     */
    Q_PROPERTY(int sortColumn READ sortColumn WRITE setSortColumn NOTIFY sortColumnChanged)

    /**
     * The number of top level rows.
     */
    Q_PROPERTY(int count READ rowCount NOTIFY rowCountChanged)

public:
    explicit KSortFilterProxyModel(QObject *parent = nullptr);
    ~KSortFilterProxyModel() override;

    void setModel(QAbstractItemModel *source);

    void setFilterRowCallback(const QJSValue &callback);
    QJSValue filterRowCallback() const;

    void setFilterString(const QString &filterString);
    QString filterString() const;

    void setFilterColumnCallback(const QJSValue &callback);
    QJSValue filterColumnCallback() const;

    void setFilterRoleName(const QString &roleName);
    QString filterRoleName() const;

    void setSortRoleName(const QString &roleName);
    QString sortRoleName() const;

    void setSortOrder(const Qt::SortOrder order);
    void setSortColumn(int column);

    void classBegin() override;
    void componentComplete() override;

public Q_SLOTS:
    /**
     * Invalidates the current filtering.
     *
     * This function should be called if you are implementing custom filtering through
     * filterRowCallback or filterColumnCallback, and your filter parameters have changed.
     *
     * @since 5.70
     */
    void invalidateFilter();

Q_SIGNALS:
    void filterStringChanged();
    void filterRoleNameChanged();
    void sortRoleNameChanged();
    void sortOrderChanged();
    void sortColumnChanged();
    void sourceModelChanged(QObject *);
    void filterRowCallbackChanged(const QJSValue &);
    void filterColumnCallbackChanged(const QJSValue &);
    void rowCountChanged();

protected:
    int roleNameToId(const QString &name) const;
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
    bool filterAcceptsColumn(int source_column, const QModelIndex &source_parent) const override;

protected Q_SLOTS:
    void syncRoleNames();

private:
    bool m_componentCompleted = false;
    QString m_filterRoleName;
    QString m_filterString;
    QString m_sortRoleName;
    QJSValue m_filterRowCallback;
    QJSValue m_filterColumnCallback;
    QHash<QString, int> m_roleIds;
};
#endif
