// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PROCESS_TABLE_MODEL_H
#define PROCESS_TABLE_MODEL_H

#include "process/process_set.h"

#include <QAbstractTableModel>
#include <QList>
#include <QMap>

// name column display
constexpr const char *kProcessName = QT_TRANSLATE_NOOP("Process.Table.Header", "Name");
// cpu column display
constexpr const char *kProcessCPU = QT_TRANSLATE_NOOP("Process.Table.Header", "CPU");
// user column display
constexpr const char *kProcessUser = QT_TRANSLATE_NOOP("Process.Table.Header", "User");
// memory column display
constexpr const char *kProcessMemory = QT_TRANSLATE_NOOP("Process.Table.Header", "Memory");
constexpr const char *kProcessShareMemory = QT_TRANSLATE_NOOP("Process.Table.Header", "Shared memory");
constexpr const char *kProcessVtrMemory = QT_TRANSLATE_NOOP("Process.Table.Header", "Virtual memory");
// upload column display
constexpr const char *kProcessUpload = QT_TRANSLATE_NOOP("Process.Table.Header", "Upload");
// download column display
constexpr const char *kProcessDownload = QT_TRANSLATE_NOOP("Process.Table.Header", "Download");
// disk read column display
constexpr const char *kProcessDiskRead = QT_TRANSLATE_NOOP("Process.Table.Header", "Disk read");
// disk write column display
constexpr const char *kProcessDiskWrite = QT_TRANSLATE_NOOP("Process.Table.Header", "Disk write");
// pid column display
constexpr const char *kProcessPID = QT_TRANSLATE_NOOP("Process.Table.Header", "PID");
// nice column display
constexpr const char *kProcessNice = QT_TRANSLATE_NOOP("Process.Table.Header", "Nice");
// priority column display
constexpr const char *kProcessPriority = QT_TRANSLATE_NOOP("Process.Table.Header", "Priority");

using namespace core::process;

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
        kProcessNameColumn = 0, // name column index
        kProcessCPUColumn, // cpu column index
        kProcessUserColumn, // user column index
        kProcessMemoryColumn, // memory column index
        kProcessShareMemoryColumn, // share memory column index
        kProcessVTRMemoryColumn, // vtr memory column index
        kProcessUploadColumn, // upload column index
        kProcessDownloadColumn, // download column index
        kProcessDiskReadColumn, // disk read column index
        kProcessDiskWriteColumn, // disk write column index
        kProcessPIDColumn, // pid column index
        kProcessNiceColumn, // nice column index
        kProcessPriorityColumn, // priority column index

        kProcessColumnCount // total number of columns
    };

    /**
     * @brief Model constructor
     * @param parent Parent object
     */
    explicit ProcessTableModel(QObject *parent = nullptr, const QString &username = nullptr);

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
    char getProcessState(pid_t pid) const;

    /**
     * @brief Get process priority enum type
     * @param pid Id of the process to get priority stub for
     * @return Process priority enum type
     */
    ProcessPriority getProcessPriority(pid_t pid) const;

    /**
     * @brief Get process priority enum type
     * @param pid Id of the process to get priority stub for
     * @return Process priority enum type
     */
    int getProcessPriorityValue(pid_t pid) const;

    /**
     * @brief Get process entry from list with specified pid
     * @param pid Process id
     * @return Process entry item
     */
    Process getProcess(pid_t pid) const;
   void setUserModeName(const QString &userName);
    qreal getTotalCPUUsage();
    qreal getTotalMemoryUsage();
    qreal getTotalDownload();
    qreal getTotalUpload();
    qreal getTotalVirtualMemoryUsage();
    qreal getTotalSharedMemoryUsage();
    qreal getTotalDiskRead();
    qreal getTotalDiskWrite();

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

    void updateProcessListWithUserSpecified();
private:
    QList<pid_t> m_procIdList; // pid list
    QList<Process> m_processList; // pid list

    QString m_userModeName {};
};

#endif  // PROCESS_TABLE_MODEL_H
