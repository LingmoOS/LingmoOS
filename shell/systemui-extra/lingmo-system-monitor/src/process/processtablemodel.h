/*
 * Copyright (C) 2021 KylinSoft Co., Ltd.
 *
 * Authors:
 *  Yang Min yangmin@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __PROCESS_TABLE_MODEL_H__
#define __PROCESS_TABLE_MODEL_H__

#include "process/process_list.h"

#include <QAbstractTableModel>
#include <QList>
#include <QMap>

using namespace sysmonitor::process;

/**
 * @brief Process table model class
 */
class ProcessTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    /**
     * @brief Process table column index
     */
    enum Column {
        ProcessNameColumn = 0, // name column index
        ProcessUserColumn, // user column index
        ProcessDiskIoColumn, // diskio column index
        ProcessCpuColumn, // cpu column index
        ProcessIdColumn, // id column index
        ProcessFlowNetColumn, // flownet column index
        ProcessMemoryColumn, // memory column index
        ProcessNiceColumn, // nice column index

        ProcessColumnCount // total number of columns
    };

    /**
     * @brief Model constructor
     * @param parent Parent object
     */
    explicit ProcessTableModel(QObject *parent = nullptr);

    /**
     * @brief Update process model with the data provided by list
     * @param list Process entry list
     */
    void updateProcessList();

    /**
     * @brief Returns the number of rows under the given parent
     * @param parent Parent index
     * @return Row count
     */
    int rowCount(const QModelIndex &parent = {}) const override;
    /**
     * @brief Returns the number of columns for the children of the given parent
     * @param parent Parent index
     * @return Column count
     */
    int columnCount(const QModelIndex &parent = {}) const override;
    /**
     * @brief Returns the data stored under the given role for the item referred to by the index
     * @param index Index of the data
     * @param role Role of the data
     * @return Data of specified role referred with index within model
     */
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    /**
     * @brief Returns the data for the given role and section in the header with the specified orientation
     * @param section Section of the header
     * @param orientation Orientation of the header
     * @param role Data role
     * @return Data of the give role & section with specified orientation within data source
     */
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    /**
     * @brief Set the data for the given role and section in the header with the specified orientation
     * @param section Section of the header
     * @param orientation Orientation of the header
     * @param value Set data
     * @param role Data role
     * @return Set result
     */
    bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role);

    /**
     * @brief Returns the item flags for the given index
     * @param index Model index to get flags for
     * @return Combination of item flags
     */
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    /**
     * @brief Get process state with specified pid
     * @param pid Id of the process to get state for
     * @return Process state string
     */
    QString getProcessState(pid_t pid) const;
    /**
     * @brief Get process priority enum type
     * @param pid Id of the process to get priority stub for
     * @return Process priority enum type
     */
    ProcessPriority getProcessPriority(pid_t pid) const;
    /**
     * @brief Get process priority with specified pid
     * @param pid Id of the process to get priority for
     * @return Process priority
     */
    int getProcessNice(pid_t pid) const;
    /**
     * @brief Get process entry from list with specified pid
     * @param pid Process id
     * @return Process entry item
     */
    sysmonitor::process::Process getProcess(pid_t pid) const;

Q_SIGNALS:
    /**
     * @brief Model updated signal
     */
    void modelUpdated();

private Q_SLOTS:
    /**
     * @brief Remove process entry from model with specified pid
     * @param pid Process id
     */
    void removeProcess(pid_t pid);
    /**
     * @brief Update the state of the process entry with specified pid
     * @param pid Process id
     * @param state Process state
     */
    void updateProcessState(pid_t pid, char state);
    /**
     * @brief Update priority of the process entry with specified pid
     * @param pid Process id
     * @param priority Process priority
     */
    void updateProcessPriority(pid_t pid, int priority);

    void updateProcessListDelay();

private:
    QList<pid_t> m_procIdList; // pid list
    QList<sysmonitor::process::Process> m_processList; // pid list
    QMap<QString,QPixmap> m_procIconMap; // icon map
    QMap<int, QString> headerDataMap;
};

#endif  // __PROCESS_TABLE_MODEL_H__
