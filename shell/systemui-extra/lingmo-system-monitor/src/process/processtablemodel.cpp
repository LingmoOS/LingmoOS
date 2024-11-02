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

#include "processtablemodel.h"
#include "../process/process_monitor.h"
#include "../util.h"

#include <QDebug>
#include <QTimer>
#include <QApplication>
#include <QFileInfo>
#include <lingmosdk/lingmosdk-base/kyutils.h>

using namespace sysmonitor::process;

//get process memory data
inline QString formatProcMemory(qreal size)
{
    enum {
        K_INDEX,
        M_INDEX,
        G_INDEX,
        T_INDEX
    };
    guint64 m_size = guint64(size);
    char data[128] = {0};
    sprintf(data,"%lu",m_size);
    char result_data[128] = {0};
    double m_speed = 0;

    QList<guint64> factorList;
    factorList.append(G_GUINT64_CONSTANT(1) << 10);//KiB
    factorList.append(G_GUINT64_CONSTANT(1) << 20);//MiB
    factorList.append(G_GUINT64_CONSTANT(1) << 30);//GiB
    factorList.append(G_GUINT64_CONSTANT(1) << 40);//TiB

    guint64 factor;
    QString format;
    factor = factorList.at(M_INDEX);
    format = QObject::tr("MiB");
    
    if (m_size >= 1024*1024 && kdkVolumeBaseCharacterConvert(data, KDK_MEGABYTE, result_data) == KDK_NOERR){
        QString formatted_result = "";
        sscanf(result_data, "%lf", &m_speed);
        formatted_result.setNum(m_speed, 'f', 1);
        return formatted_result + format;
    } else {
        size = size / (double)factor;
        return QString("%1 %2").arg(size, 0, 'f', 1).arg(format);
    }
}

// model constructor
ProcessTableModel::ProcessTableModel(QObject *parent)
    : QAbstractTableModel(parent)
{
    //update model's process list cache on process list updated signal
    auto *monitor = ProcessMonitor::instance();
    connect(monitor, &ProcessMonitor::procInfoUpdated, this, &ProcessTableModel::updateProcessList);

    //remove process entry from model's cache on process ended signal
    connect(monitor->processList(), &ProcessList::processEnded, this, &ProcessTableModel::removeProcess);
    //update process's state in model's cache on process paused signal
    connect(monitor->processList(), &ProcessList::processPaused, this,
            &ProcessTableModel::updateProcessState);
    //update process's state in model's cache on process resumed signal
    connect(monitor->processList(), &ProcessList::processResumed, this,
            &ProcessTableModel::updateProcessState);
    //remove process entry from model's cache on process killed signal
    connect(monitor->processList(), &ProcessList::processKilled, this,
            &ProcessTableModel::removeProcess);
    //update process's priority in model's cache on process priority changed signal
    connect(monitor->processList(), &ProcessList::processPriorityChanged, this,
            &ProcessTableModel::updateProcessPriority);

    m_procIconMap.clear();
}

QString ProcessTableModel::getProcessState(pid_t pid) const
{
    if (m_procIdList.contains(pid)) {
        sysmonitor::process::Process proInfo = ProcessMonitor::instance()->processList()->getProcessById(pid);
        if (proInfo.isValid()) {
            return proInfo.getProcStatus();
        }
        return "";
    }

    return "";
}

int ProcessTableModel::getProcessNice(pid_t pid) const
{
    if (m_procIdList.contains(pid)) {
        sysmonitor::process::Process proInfo = ProcessMonitor::instance()->processList()->getProcessById(pid);
        if (proInfo.isValid()) {
            return proInfo.getNice();
        }
        return InvalidPriority;
    }

    return InvalidPriority;
}

sysmonitor::process::Process ProcessTableModel::getProcess(pid_t pid) const
{
    if (m_procIdList.contains(pid)) {
        return ProcessMonitor::instance()->processList()->getProcessById(pid);
    }

    return sysmonitor::process::Process();
}

// update process model with the data provided by list
void ProcessTableModel::updateProcessList()
{
    QTimer::singleShot(0, this, SLOT(updateProcessListDelay()));
}

void ProcessTableModel::updateProcessListDelay()
{
    ProcessList *processList = ProcessMonitor::instance()->processList();
    const QList<pid_t> &newpidlst = processList->getPIDList();
    QList<pid_t> oldpidlst = m_procIdList;

    for (const auto &pid : newpidlst) {
        int row = m_procIdList.indexOf(pid);
        if (row >= 0) {
            // update
            m_processList[row] = processList->getProcessById(pid);
            QString strIconPath;
            QPixmap iconPixmap = processList->getProcIcon(pid, strIconPath, 24);
            m_processList[row].setIconPath(strIconPath);
            m_procIconMap[strIconPath] = iconPixmap;
            Q_EMIT dataChanged(index(row, 0), index(row, columnCount() - 1));
        } else {
            // insert
            row = m_procIdList.size();
            beginInsertRows({}, row, row);
            m_procIdList << pid;
            m_processList << processList->getProcessById(pid);
            QString strIconPath;
            QPixmap iconPixmap = processList->getProcIcon(pid, strIconPath, 24);
            m_processList[row].setIconPath(strIconPath);
            m_procIconMap[strIconPath] = iconPixmap;
            endInsertRows();
        }
    }

    // remove
    for (const auto &pid : oldpidlst) {
        if (!newpidlst.contains(pid)) {
            int row = m_procIdList.indexOf(pid);
            beginRemoveRows({}, row, row);
            m_procIdList.removeAt(row);
            m_processList.removeAt(row);
            endRemoveRows();
        }
    }

    Q_EMIT modelUpdated();
}

// returns the number of rows under the given parent
int ProcessTableModel::rowCount(const QModelIndex &) const
{
    return m_procIdList.size();
}

// returns the number of columns for the children of the given parent
int ProcessTableModel::columnCount(const QModelIndex &) const
{
    return ProcessColumnCount;
}

bool ProcessTableModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role)
{
    if (section == -1) {
        headerDataMap.clear();
        return false;
    } else {
        headerDataMap[section] = value.toString();
        return QAbstractTableModel::setHeaderData(section, orientation, value, role);
    }
}

// returns the data for the given role and section in the header with the specified orientation
QVariant ProcessTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole || role == Qt::AccessibleTextRole) {
        if (headerDataMap.keys().contains(section)) {
            return headerDataMap[section];
        }
        switch (section) {
        case ProcessNameColumn: {
            // name column display text
            return tr("Process Name");
        }
        case ProcessUserColumn: {
            // user column display text
            return tr("User");
        }
        case ProcessDiskIoColumn:
            // diskio column display text
            return tr("Disk");
        case ProcessCpuColumn:
            // cpu column display text
            return tr("CPU");
        case ProcessIdColumn:
            // id column display text
            return tr("ID");
        case ProcessFlowNetColumn:
            // flownet column display text
            return tr("Flownet Persec");
        case ProcessMemoryColumn:
            // memory column display text
            return tr("Memory");
        case ProcessNiceColumn:
            // priority column display text
            return tr("Priority");
        default:
            break;
        }
    } else if (role == Qt::TextAlignmentRole) {
        // default header section alignment
        return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
    } else if (role == Qt::InitialSortOrderRole) {
        // sort section descending by default
        return QVariant::fromValue(Qt::DescendingOrder);
    }
    return QAbstractTableModel::headerData(section, orientation, role);
}

// returns the data stored under the given role for the item referred to by the index
QVariant ProcessTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return {};

    // validate index
    if (index.row() < 0 || index.row() >= m_processList.size())
        return {};

    int row = index.row();
    const sysmonitor::process::Process &proc = m_processList[row];
    if (!proc.isValid())
        return {};
    if (role == Qt::DisplayRole || role == Qt::AccessibleTextRole) {
        switch (index.column()) {
        case ProcessNameColumn: {
            // prepended tag based on process state
            QString name = proc.getDisplayName();
            if (proc.getProcStatus() == QObject::tr("Stopped")) {//已停止
                name = QString("(%1) %2").arg(tr("Suspend")).arg(name);
            }
            else if (proc.getProcStatus() == QObject::tr("Zombie")) {//僵死
                name = QString("(%1) %2").arg(tr("No response")).arg(name);
            }
            else if (proc.getProcStatus() == QObject::tr("Uninterruptible")) {//不可中断
                name = QString("(%1) %2").arg(tr("Uninterruptible")).arg(name);
            }
            else {//Sleeping 睡眠中  Running 运行中
            }
            return name;
        }
        case ProcessUserColumn:
            // process's user name
            return proc.getProcUser();
        case ProcessDiskIoColumn:
            // process's diskio desc
            return proc.getDiskIODesc();
        case ProcessCpuColumn:
            // formated cpu percent utilization
            return QString("%1%").arg(proc.getCpuPercent(), 0, 'f', 1);
        case ProcessIdColumn: 
            // process pid text
            return QString("%1").arg(proc.pid());
        case ProcessFlowNetColumn:
            // flownet speed text
            return proc.getFlowNetDesc();
        case ProcessMemoryColumn:
            // formatted memory usage
            return formatProcMemory(proc.getMemeryUsed());
        case ProcessNiceColumn:
            // process priority enum text representation
            return getNiceLevel(proc.getNice());
        default:
            break;
        }
    } else if (role == Qt::DecorationRole) {
        switch (index.column()) {
        case ProcessNameColumn:
        {
            // process icon
            if (m_procIconMap.contains(proc.getIconPath())) {
                return m_procIconMap[proc.getIconPath()];
            }
            break;
        }
        default:
            return {};
        }
    } else if (role == Qt::UserRole) {
        // get process's raw data
        switch (index.column()) {
        case ProcessNameColumn:
            return proc.getProcName();
        case ProcessDiskIoColumn:
            return proc.getDiskIO();
        case ProcessCpuColumn:
            return proc.getCpuPercent();
        case ProcessIdColumn:
            return proc.pid();
        case ProcessFlowNetColumn:
            return proc.getFlowNet();
        case ProcessMemoryColumn:
            return proc.getMemeryUsed();
        case ProcessNiceColumn:
            return proc.getNice();
        default:
            return {};
        }
    } else if (role == (Qt::UserRole + 1)) {
        // get process's extra data
        switch (index.column()) {
        case ProcessFlowNetColumn:
            return proc.getFlowNet();
        case ProcessNameColumn:
            return proc.getDisplayName();
        default:
            return {};
        }
    } else if (role == Qt::TextAlignmentRole) {
        // default data alignment
        return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
    } else if (role == Qt::ForegroundRole) {
        switch (index.column()) {
        case ProcessNameColumn: {
            // prepended tag based on process state
            QString name = proc.getDisplayName();
            if (proc.getProcStatus() == QObject::tr("Stopped")) {//已停止
                return QVariant(QColor("#fca71d"));
            }
            else if (proc.getProcStatus() == QObject::tr("Zombie")) {//僵死
                return QVariant(QColor("#808080"));
            }
            else if (proc.getProcStatus() == QObject::tr("Uninterruptible")) {//不可中断
                return QVariant(QColor("#ff6a6a"));
            }
            else {//Sleeping 睡眠中  Running 运行中
            }
        }
        default:
            break;
        }
    } else if (role == Qt::UserRole + 2) {
        // text color role based on process's state
        if (index.column() == ProcessNameColumn) {
        }
        return {};
    } else if (role == Qt::UserRole + 3) {
        return proc.getProcType();
    }
    return {};
}

// returns the item flags for the given index
Qt::ItemFlags ProcessTableModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

// get process priority enum type
ProcessPriority ProcessTableModel::getProcessPriority(pid_t pid) const
{
    int row = m_procIdList.indexOf(pid);
    if (row >= 0) {
        sysmonitor::process::Process procInfo = ProcessMonitor::instance()->processList()->getProcessById(pid);
        if (procInfo.isValid()) {
            return getProcessPriorityStub(procInfo.getNice());
        }
        return InvalidPriority;
    }

    return InvalidPriority;
}

// remove process entry from model with specified pid
void ProcessTableModel::removeProcess(pid_t pid)
{
    int row = m_procIdList.indexOf(pid);
    if (row >= 0) {
        beginRemoveRows(QModelIndex(), row, row);
        m_procIdList.removeAt(row);
        m_processList.removeAt(row);
        endRemoveRows();
    }
}

// update the state of the process entry with specified pid
void ProcessTableModel::updateProcessState(pid_t pid, char state)
{
    Q_UNUSED(pid);
    Q_UNUSED(state);
    /*int row = m_procIdList.indexOf(pid);
    if (row >= 0) {
        m_processList[row].setProcStatus(state);
        Q_EMIT dataChanged(index(row, 0), index(row, columnCount() - 1));
    }*/
}

// update priority of the process entry with specified pid
void ProcessTableModel::updateProcessPriority(pid_t pid, int priority)
{
    int row = m_procIdList.indexOf(pid);
    if (row >= 0) {
        m_processList[row].setNice(priority);
        Q_EMIT dataChanged(index(row, 0), index(row, columnCount() - 1));
    }
}
