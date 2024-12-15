// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SYSTEM_SERVICE_TABLE_MODEL_H
#define SYSTEM_SERVICE_TABLE_MODEL_H

#include "service/system_service_entry.h"

#include <QAbstractTableModel>
#include <QList>
#include <QHash>

// Service name text
constexpr const char *kSystemServiceName = QT_TRANSLATE_NOOP("Service.Table.Header", "Name");
// Service load state text
constexpr const char *kSystemServiceLoadState = QT_TRANSLATE_NOOP("Service.Table.Header", "Load");
// Service active state text
constexpr const char *kSystemServiceActiveState =
    QT_TRANSLATE_NOOP("Service.Table.Header", "Start State");
// Service sub state (running status) text
constexpr const char *kSystemServiceSubState = QT_TRANSLATE_NOOP("Service.Table.Header", "Sub");
// Service state text
constexpr const char *kSystemServiceState = QT_TRANSLATE_NOOP("Service.Table.Header", "Start Tactic");
// Service startup mode text
constexpr const char *kSystemServiceStartupMode = QT_TRANSLATE_NOOP("Service.Table.Header", "Startup Type");
// Service description text
constexpr const char *kSystemServiceDescription =
    QT_TRANSLATE_NOOP("Service.Table.Header", "Description");
// Service pid text
constexpr const char *kSystemServicePID = QT_TRANSLATE_NOOP("Service.Table.Header", "PID");

class SystemServiceEntry;

/**
 * @brief System service table model
 */
class SystemServiceTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    /**
     * @brief Service table column index enum
     */
    enum SystemServiceTableColumn {
        kSystemServiceNameColumn = 0, // name column
        kSystemServiceLoadStateColumn, // load state column
        kSystemServiceActiveStateColumn, // active state column
        kSystemServiceSubStateColumn, // sub state column
        kSystemServiceStateColumn, // state column
        kSystemServiceDescriptionColumn, // description column
        kSystemServicePIDColumn, // pid column
        kSystemServiceStartupModeColumn, // startup mode column

        kSystemServiceTableColumnCount // total number of columns
    };

    /**
     * @brief Model constructor
     * @param parent Parent object
     */
    explicit SystemServiceTableModel(QObject *parent = nullptr);

    /**
     * @brief Reset model
     */
    void reset();

    /**
     * @brief Update model with source data provided by entry
     * @param entry Model update source entry
     */
    void updateServiceEntry(const SystemServiceEntry &entry);

    /**
     * @brief Get unit file's state
     * @param index Model index
     * @return Return unit file's state string
     */
    QString getUnitFileState(const QModelIndex &index);

    /**
     * @brief Get unit file's name string
     * @param index Model index
     * @return Return unit file's name string
     */
    QString getUnitFileName(const QModelIndex &index);

    /**
     * @brief Get unit active's state
     * @param index Model index
     * @return Return unit active's state string
     */
    QString getUnitActiveState(const QModelIndex &index);

    /**
     * @brief data Returns the data stored under the given role for the item referred to by the index
     * @param index Index of the data
     * @param role Role of the data
     * @return Data of specified role referred with index within model
     */
    QVariant data(const QModelIndex &index, int role) const override;
    /**
     * @brief Returns the number of rows under the given parent
     * @param parent Parent index
     * @return Row count
     */
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    /**
     * @brief Returns the number of columns for the children of the given parent
     * @param parent Parent index
     * @return Column count
     */
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    /**
     * @brief Returns the data for the given role and section in the header with the specified orientation
     * @param section Section of the header
     * @param orientation Orientation of the header
     * @param role Data role
     * @return Data of the give role & section with specified orientation within data source
     */
    QVariant headerData(int section,
                        Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;
    /**
     * @brief Returns the item flags for the given index
     * @param index Model index to get flags for
     * @return Combination of item flags
     */
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    /**
     * @brief Fetches any available data for the items with the parent specified by the parent index
     * @param parent Parent index
     */
    void fetchMore(const QModelIndex &parent) override;
    /**
     * @brief Check if more data can be fetched for parent index
     * @param parent Parent index
     * @return Returns true if there is more data available for parent; otherwise returns false.
     */
    bool canFetchMore(const QModelIndex &parent) const override;

Q_SIGNALS:
    /**
     * @brief Current itme's row number changed signal
     * @param row Row number of current item
     */
    void currentRowChanged(int row);

private Q_SLOTS:
    /**
     * @brief Update the model with the data provided by list
     * @param list Updated service's list
     */
    void updateServiceList(const QList<SystemServiceEntry> &list);

private:
    // Service name list
    QList<QString>                      m_svcList   {};
    // Service name - entry mapping
    QHash<QString, SystemServiceEntry>  m_svcMap    {};
    // current loaded items (into the model)
    int m_nr {};
};

inline void SystemServiceTableModel::reset()
{
    beginRemoveRows({}, 0, m_svcList.size() - 1);
    m_svcList.clear();
    m_svcMap.clear();
    endRemoveRows();
}

inline QString SystemServiceTableModel::getUnitFileState(const QModelIndex &index)
{
    if (!index.isValid())
        return {};

    return m_svcMap[m_svcList[index.row()]].getState();
}

inline QString SystemServiceTableModel::getUnitFileName(const QModelIndex &index)
{
    if (!index.isValid())
        return {};

    return m_svcMap[m_svcList[index.row()]].getSName();
}

inline QString SystemServiceTableModel::getUnitActiveState(const QModelIndex &index)
{
    if (!index.isValid())
        return {};

    return m_svcMap[m_svcList[index.row()]].getActiveState();
}

#endif  // SYSTEM_SERVICE_TABLE_MODEL_H
