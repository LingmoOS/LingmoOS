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

#include "ksortfilterproxymodel.h"

#include <QQmlContext>
#include <QQmlEngine>

#include "kitemmodels_debug.h"

KSortFilterProxyModel::KSortFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
    , m_componentCompleted(false)
    , m_sortRoleSourceOfTruth(SourceOfTruthIsRoleID)
    , m_filterRoleSourceOfTruth(SourceOfTruthIsRoleID)
    , m_sortRoleGuard(false)
    , m_filterRoleGuard(false)
{
    setDynamicSortFilter(true);
    connect(this, &KSortFilterProxyModel::modelReset, this, &KSortFilterProxyModel::rowCountChanged);
    connect(this, &KSortFilterProxyModel::rowsInserted, this, &KSortFilterProxyModel::rowCountChanged);
    connect(this, &KSortFilterProxyModel::rowsRemoved, this, &KSortFilterProxyModel::rowCountChanged);

    connect(this, &KSortFilterProxyModel::sortRoleChanged, this, &KSortFilterProxyModel::syncSortRoleProperties);
    connect(this, &KSortFilterProxyModel::filterRoleChanged, this, &KSortFilterProxyModel::syncFilterRoleProperties);
}

KSortFilterProxyModel::~KSortFilterProxyModel()
{
}

static void reverseStringIntHash(QHash<QString, int> &dst, const QHash<int, QByteArray> &src)
{
    dst.clear();
    dst.reserve(src.count());
    for (auto i = src.constBegin(); i != src.constEnd(); ++i) {
        dst[QString::fromUtf8(i.value())] = i.key();
    }
}

void KSortFilterProxyModel::syncRoleNames()
{
    if (!sourceModel()) {
        return;
    }

    reverseStringIntHash(m_roleIds, roleNames());

    m_sortRoleGuard = true;
    syncSortRoleProperties();
    m_sortRoleGuard = false;

    m_filterRoleGuard = true;
    syncFilterRoleProperties();
    m_filterRoleGuard = false;
}

int KSortFilterProxyModel::roleNameToId(const QString &name) const
{
    return m_roleIds.value(name, Qt::DisplayRole);
}

void KSortFilterProxyModel::setSourceModel(QAbstractItemModel *model)
{
    const auto oldModel = sourceModel();

    if (model == oldModel) {
        return;
    }

    if (oldModel) {
        for (const auto &connection : std::as_const(m_sourceModelConnections)) {
            disconnect(connection);
        }
    }

    QSortFilterProxyModel::setSourceModel(model);

    // NOTE: some models actually fill their roleNames() only when they get some actual data, this works around the bad behavior
    if (model) {
        m_sourceModelConnections = {{
            connect(model, &QAbstractItemModel::modelReset, this, &KSortFilterProxyModel::syncRoleNames),
            connect(model, &QAbstractItemModel::rowsInserted, this, &KSortFilterProxyModel::syncRoleNames),
            connect(model, &QAbstractItemModel::rowsRemoved, this, &KSortFilterProxyModel::syncRoleNames),
        }};
    }

    if (m_componentCompleted) {
        syncRoleNames();
    }
}

bool KSortFilterProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    if (m_filterRowCallback.isCallable()) {
        QJSEngine *engine = qjsEngine(this);
        QJSValueList args = {QJSValue(source_row), engine->toScriptValue(source_parent)};

        QJSValue result = const_cast<KSortFilterProxyModel *>(this)->m_filterRowCallback.call(args);
        if (result.isError()) {
            qCWarning(KITEMMODELS_LOG) << "Row filter callback produced an error:";
            qCWarning(KITEMMODELS_LOG) << result.toString();
            return true;
        } else {
            return result.toBool();
        }
    }

    return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
}

bool KSortFilterProxyModel::filterAcceptsColumn(int source_column, const QModelIndex &source_parent) const
{
    if (m_filterColumnCallback.isCallable()) {
        QJSEngine *engine = qjsEngine(this);
        QJSValueList args = {QJSValue(source_column), engine->toScriptValue(source_parent)};

        QJSValue result = const_cast<KSortFilterProxyModel *>(this)->m_filterColumnCallback.call(args);
        if (result.isError()) {
            qCWarning(KITEMMODELS_LOG) << "Row filter callback produced an error:";
            qCWarning(KITEMMODELS_LOG) << result.toString();
            return true;
        } else {
            return result.toBool();
        }
    }

    return QSortFilterProxyModel::filterAcceptsColumn(source_column, source_parent);
}

void KSortFilterProxyModel::setFilterString(const QString &filterString)
{
    if (filterString == m_filterString) {
        return;
    }
    m_filterString = filterString;
    QSortFilterProxyModel::setFilterFixedString(filterString);
    Q_EMIT filterStringChanged();
}

QString KSortFilterProxyModel::filterString() const
{
    return m_filterString;
}

QJSValue KSortFilterProxyModel::filterRowCallback() const
{
    return m_filterRowCallback;
}

void KSortFilterProxyModel::setFilterRowCallback(const QJSValue &callback)
{
    if (m_filterRowCallback.strictlyEquals(callback)) {
        return;
    }

    if (!callback.isNull() && !callback.isCallable()) {
        return;
    }

    m_filterRowCallback = callback;
    invalidateFilter();

    Q_EMIT filterRowCallbackChanged(callback);
}

void KSortFilterProxyModel::setFilterColumnCallback(const QJSValue &callback)
{
    if (m_filterColumnCallback.strictlyEquals(callback)) {
        return;
    }

    if (!callback.isNull() && !callback.isCallable()) {
        return;
    }

    m_filterColumnCallback = callback;
    invalidateFilter();

    Q_EMIT filterColumnCallbackChanged(callback);
}

QJSValue KSortFilterProxyModel::filterColumnCallback() const
{
    return m_filterColumnCallback;
}

void KSortFilterProxyModel::syncSortRoleProperties()
{
    if (!sourceModel()) {
        return;
    }

    if (!m_sortRoleGuard) {
        m_sortRoleSourceOfTruth = SourceOfTruthIsRoleID;
    }

    if (m_sortRoleSourceOfTruth == SourceOfTruthIsRoleName) {
        if (m_sortRoleName.isEmpty()) {
            QSortFilterProxyModel::setSortRole(Qt::DisplayRole);
            sort(-1, Qt::AscendingOrder);
        } else {
            const auto role = roleNameToId(m_sortRoleName);
            QSortFilterProxyModel::setSortRole(role);
            sort(std::max(sortColumn(), 0), sortOrder());
        }
    } else {
        const QString roleName = QString::fromUtf8(roleNames().value(sortRole()));
        if (m_sortRoleName != roleName) {
            m_sortRoleName = roleName;
            Q_EMIT sortRoleNameChanged();
        }
    }
}

void KSortFilterProxyModel::syncFilterRoleProperties()
{
    if (!sourceModel()) {
        return;
    }

    if (!m_filterRoleGuard) {
        m_filterRoleSourceOfTruth = SourceOfTruthIsRoleID;
    }

    if (m_filterRoleSourceOfTruth == SourceOfTruthIsRoleName) {
        const auto role = roleNameToId(m_filterRoleName);
        QSortFilterProxyModel::setFilterRole(role);
    } else {
        const QString roleName = QString::fromUtf8(roleNames().value(filterRole()));
        if (m_filterRoleName != roleName) {
            m_filterRoleName = roleName;
            Q_EMIT filterRoleNameChanged();
        }
    }
}

void KSortFilterProxyModel::setFilterRoleName(const QString &roleName)
{
    if (m_filterRoleSourceOfTruth == SourceOfTruthIsRoleName && m_filterRoleName == roleName) {
        return;
    }

    m_filterRoleSourceOfTruth = SourceOfTruthIsRoleName;
    m_filterRoleName = roleName;

    m_filterRoleGuard = true;
    syncFilterRoleProperties();
    m_filterRoleGuard = false;

    Q_EMIT filterRoleNameChanged();
}

QString KSortFilterProxyModel::filterRoleName() const
{
    return m_filterRoleName;
}

void KSortFilterProxyModel::setSortRoleName(const QString &roleName)
{
    if (m_sortRoleSourceOfTruth == SourceOfTruthIsRoleName && m_sortRoleName == roleName) {
        return;
    }

    m_sortRoleSourceOfTruth = SourceOfTruthIsRoleName;
    m_sortRoleName = roleName;

    m_sortRoleGuard = true;
    syncSortRoleProperties();
    m_sortRoleGuard = false;

    Q_EMIT sortRoleNameChanged();
}

QString KSortFilterProxyModel::sortRoleName() const
{
    return m_sortRoleName;
}

void KSortFilterProxyModel::setSortOrder(const Qt::SortOrder order)
{
    sort(std::max(sortColumn(), 0), order);
    Q_EMIT sortOrderChanged();
}

void KSortFilterProxyModel::setSortColumn(int column)
{
    if (column == sortColumn()) {
        return;
    }
    sort(column, sortOrder());
    Q_EMIT sortColumnChanged();
}

void KSortFilterProxyModel::classBegin()
{
}

void KSortFilterProxyModel::componentComplete()
{
    m_componentCompleted = true;
    syncRoleNames();
}

void KSortFilterProxyModel::invalidateFilter()
{
    QSortFilterProxyModel::invalidateFilter();
}

#include "moc_ksortfilterproxymodel.cpp"
