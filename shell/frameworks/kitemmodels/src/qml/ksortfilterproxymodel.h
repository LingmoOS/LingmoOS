/*
 *   Copyright 2010 by Marco Martin <mart@kde.org>
 *   Copyright 2019 by David Edmundson <davidedmundson@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef KSORTFILTERPROXYMODEL_H
#define KSORTFILTERPROXYMODEL_H

#include <QAbstractItemModel>
#include <QJSValue>
#include <QList>
#include <QQmlParserStatus>
#include <QSortFilterProxyModel>
#include <qqmlregistration.h>

#include <array>

/**
 * @class KSortFilterProxyModel
 * @short Filter and sort an existing QAbstractItemModel
 *
 * @since 5.67
 */
class KSortFilterProxyModel : public QSortFilterProxyModel, public QQmlParserStatus
{
    Q_OBJECT
    QML_ELEMENT
    Q_INTERFACES(QQmlParserStatus)

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

    void setSourceModel(QAbstractItemModel *sourceModel) override;

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
    void filterRowCallbackChanged(const QJSValue &);
    void filterColumnCallbackChanged(const QJSValue &);
    void rowCountChanged();

protected:
    int roleNameToId(const QString &name) const;
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
    bool filterAcceptsColumn(int source_column, const QModelIndex &source_parent) const override;

protected Q_SLOTS:
    // This method is called whenever we suspect that role names mapping might have gone stale.
    // It must not alter the source of truth for sort/filter properties.
    void syncRoleNames();
    // These methods are dealing with individual pairs of properties. They are
    // called on various occasions, and need to check whether the invocation
    // has been caused by a standalone base type's property change
    // (switching source of truth to role ID) or as a side-effect of a sync.
    void syncSortRoleProperties();
    void syncFilterRoleProperties();

private:
    // conveniently, role ID is the default source of truth, turning it into
    // zero-initialization.
    enum SourceOfTruthForRoleProperty : bool {
        SourceOfTruthIsRoleID = false,
        SourceOfTruthIsRoleName = true,
    };

    bool m_componentCompleted : 1;
    SourceOfTruthForRoleProperty m_sortRoleSourceOfTruth : 1;
    SourceOfTruthForRoleProperty m_filterRoleSourceOfTruth : 1;
    bool m_sortRoleGuard : 1;
    bool m_filterRoleGuard : 1;
    // default role name corresponds to the standard mapping of the default Qt::DisplayRole in QAbstractItemModel::roleNames
    QString m_sortRoleName{QStringLiteral("display")};
    QString m_filterRoleName{QStringLiteral("display")};
    QString m_filterString;
    QJSValue m_filterRowCallback;
    QJSValue m_filterColumnCallback;
    QHash<QString, int> m_roleIds;
    std::array<QMetaObject::Connection, 3> m_sourceModelConnections;
};

#endif
