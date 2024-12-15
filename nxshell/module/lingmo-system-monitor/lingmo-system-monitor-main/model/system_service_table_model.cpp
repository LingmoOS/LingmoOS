// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "system_service_table_model.h"

#include "service/service_manager.h"
#include "common/common.h"

#include <DApplication>

#include <QColor>
#include <QDebug>
#include <QFont>
#include <QFontMetrics>

DWIDGET_USE_NAMESPACE
using namespace common;

// model constructor
SystemServiceTableModel::SystemServiceTableModel(QObject *parent)
    : QAbstractTableModel(parent)
{
    // service manager instance
    auto *mgr = ServiceManager::instance();
    Q_ASSERT(mgr);
    // conenct service list & status update slots
    connect(mgr, &ServiceManager::serviceListUpdated, this, &SystemServiceTableModel::updateServiceList);
    connect(mgr, &ServiceManager::serviceStatusUpdated, this, &SystemServiceTableModel::updateServiceEntry);
}

// update the model with the data provided by entry
void SystemServiceTableModel::updateServiceEntry(const SystemServiceEntry &entry)
{
    // get entry's service name
    auto sname = entry.getSName();
    if (m_svcMap.contains(sname)) {
        // replace the entry within model if already exists with same name
        for (auto row = 0; row < m_svcList.size(); row++) {
            if (m_svcList[row] == sname) {
                m_svcMap[sname] = entry;
                Q_EMIT dataChanged(index(row, 0), index(row, columnCount() - 1));
                break;
            }
        }
    } else {
        // when we create empty service we need jump this error service
        if (sname.isEmpty())
            return;
        // otherwise add the entry to the model
        auto row = m_svcList.size();
        beginInsertRows({}, row, row);
        m_svcList << sname;
        m_svcMap[sname] = entry;
        ++m_nr;
        endInsertRows();
        Q_EMIT currentRowChanged(row);
    }
}

// Returns the data stored under the given role for the item referred to by the index
QVariant SystemServiceTableModel::data(const QModelIndex &index, int role) const
{
    QString buf {};
    auto row = index.row();

    // check index validity
    if (!index.isValid() || !(row >= 0 && row < m_svcList.size())) {
        return {};
    }

    if (role == Qt::DisplayRole || role == Qt::AccessibleTextRole) {
        switch (index.column()) {
        case kSystemServiceNameColumn:
            // get service name
            return m_svcMap[m_svcList[row]].getSName();
        case kSystemServiceLoadStateColumn: {
            // get service load state
            buf = m_svcMap[m_svcList[row]].getLoadState();
            return DApplication::translate("DBus.Unit.Load.State", buf.toUtf8());
        }
        case kSystemServiceActiveStateColumn: {
            // get service active state
            buf = m_svcMap[m_svcList[row]].getActiveState();
            return DApplication::translate("DBus.Unit.Active.State", buf.toUtf8());
        }
        case kSystemServiceSubStateColumn:
            // get service sub state
            return m_svcMap[m_svcList[row]].getSubState();
        case kSystemServiceStateColumn: {
            // get service state
            buf = m_svcMap[m_svcList[row]].getState();
            return DApplication::translate("DBus.Unit.State", buf.toUtf8());
        }
        case kSystemServiceStartupModeColumn: {
            // get service startup type
            auto stype = m_svcMap[m_svcList[row]].getStartupType();
            return DApplication::translate("DBus.Unit.Startup.Mode", stype.toUtf8());
        }
        case kSystemServiceDescriptionColumn:
            // get servcie description
            return m_svcMap[m_svcList[row]].getDescription();
        case kSystemServicePIDColumn: {
            // get main pid of the service
            auto pid = m_svcMap[m_svcList[row]].getMainPID();
            auto va = (pid == 0) ? QVariant() : QVariant(pid);
            return va;
        }
        default:
            break;
        }
    } else if (role == Qt::TextAlignmentRole) {
        // default text alignment
        return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
    }
    // return null for other data roles
    return {};
}

// Returns the number of rows under the given parent
int SystemServiceTableModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return m_nr;
}
// Returns the number of columns for the children of the given parent
int SystemServiceTableModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return kSystemServiceTableColumnCount;
}
// Returns the data for the given role and section in the header with the specified orientation
QVariant SystemServiceTableModel::headerData(int section,
                                             Qt::Orientation orientation,
                                             int role) const
{
    if (role == Qt::DisplayRole || role == Qt::AccessibleTextRole) {
        switch (section) {
        case kSystemServiceNameColumn:
            // service name column display text
            return DApplication::translate("Service.Table.Header", kSystemServiceName);
        case kSystemServiceLoadStateColumn:
            // service load state column display text
            return DApplication::translate("Service.Table.Header", kSystemServiceLoadState);
        case kSystemServiceActiveStateColumn:
            // service active state column display text
            return DApplication::translate("Service.Table.Header", kSystemServiceActiveState);
        case kSystemServiceSubStateColumn:
            // service sub state column display text
            return DApplication::translate("Service.Table.Header", kSystemServiceSubState);
        case kSystemServiceStateColumn:
            // service state column display text
            return DApplication::translate("Service.Table.Header", kSystemServiceState);
        case kSystemServiceDescriptionColumn:
            // service description column display text
            return DApplication::translate("Service.Table.Header", kSystemServiceDescription);
        case kSystemServicePIDColumn:
            // service service pid column display text
            return DApplication::translate("Service.Table.Header", kSystemServicePID);
        case kSystemServiceStartupModeColumn:
            // service startup mode column display text
            return DApplication::translate("Service.Table.Header", kSystemServiceStartupMode);
        default:
            break;
        }
    } else if (role == Qt::TextAlignmentRole) {
        // default alignment of header sections
        return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
    } else if (role == Qt::InitialSortOrderRole) {
        // sort descending by default
        return QVariant::fromValue(Qt::DescendingOrder);
    }

    // default handler for other role types
    return QAbstractTableModel::headerData(section, orientation, role);
}

// Returns the item flags for the given index
Qt::ItemFlags SystemServiceTableModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

// Fetches any available data for the items with the parent specified by the parent index
void SystemServiceTableModel::fetchMore(const QModelIndex &parent)
{
    if (parent.isValid())
        return;

    // fetch at most 100 items at a time
    int left = m_svcList.size() - m_nr;
    int more = qMin(100, left);

    if (more <= 0)
        return;

    beginInsertRows(QModelIndex(), m_nr, m_nr + more - 1);
    m_nr += more;
    endInsertRows();
}

// Check if more data can be fetched for parent index
bool SystemServiceTableModel::canFetchMore(const QModelIndex &parent) const
{
    if (parent.isValid())
        return false;

    return (m_nr < m_svcList.size());
}

// Update the model with the data provided by list
void SystemServiceTableModel::updateServiceList(const QList<SystemServiceEntry> &list)
{
    beginResetModel();
    // reset
    m_svcList.clear();
    m_svcMap.clear();
    m_nr = 0;
    // feed with new data from list
    for (auto &ent : list) {
        // when we create empty service we need jump this error service
        if (ent.getSName().isEmpty())
            continue;
        m_svcList << ent.getSName();
        m_svcMap[ent.getSName()] = ent;
    }
    endResetModel();
}
