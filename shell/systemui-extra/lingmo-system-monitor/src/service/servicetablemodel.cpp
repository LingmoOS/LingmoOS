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

#include "servicetablemodel.h"
#include "servicemanager.h"
#include "../util.h"

#include <QDebug>
#include <QTimer>
#include <QApplication>
#include <QFileInfo>

ServiceTableModel::ServiceTableModel(QObject *parent)
    : QAbstractTableModel(parent)
{
    //服务管理实例
    auto *manager = ServiceManager::instance();
    Q_ASSERT(manager);
    // 连接服务列表或单个服务更新信号
    connect(manager, &ServiceManager::serviceListUpdated, this, &ServiceTableModel::updateServiceList);
    connect(manager, &ServiceManager::serviceSingleUpdated, this, &ServiceTableModel::updateServiceInfo);

    m_listStateTr<<tr("stub")<<tr("loaded")<<tr("not-found")<<tr("bad-setting")<<tr("error")<<tr("merged")<<tr("masked")
        <<tr("active")<<tr("reloading")<<tr("inactive")<<tr("failed")<<tr("activating")<<tr("deactivating")
        <<tr("dead")<<tr("start-pre")<<tr("start")<<tr("start-post")<<tr("running")<<tr("exited")<<tr("reload")<<tr("stop")<<tr("stop-watchdog")<<tr("stop-sigterm")
            <<tr("stop-sigkill")<<tr("stop-post")<<tr("final-sigterm")<<tr("final-sigkill")<<tr("stop-sigterm")<<tr("auto-restart")
        <<tr("enabled")<<tr("disabled")<<tr("static")<<tr("transient")<<tr("indirect")<<tr("enabled-runtime")<<tr("masked")<<tr("generated")
        <<tr("Auto")<<tr("Manual")<<tr("N/A");
}

// 获取服务状态
QString ServiceTableModel::getServiceState(const QModelIndex &index) const
{
    if (!index.isValid())
        return "";

    return m_svcInfoList[m_svcNameList[index.row()]].getState();
}

// 获取服务名称
QString ServiceTableModel::getServiceName(const QModelIndex &index) const
{
    if (!index.isValid())
        return "";

    return m_svcInfoList[m_svcNameList[index.row()]].getSName();
}

// 获取服务描述
QString ServiceTableModel::getServiceDescription(const QModelIndex &index) const
{
    if (!index.isValid())
        return "";

    return m_svcInfoList[m_svcNameList[index.row()]].getDescription();
}

// 获取服务活动状态
QString ServiceTableModel::getServiceActiveState(const QModelIndex &index) const
{
    if (!index.isValid())
        return "";

    return m_svcInfoList[m_svcNameList[index.row()]].getActiveState();
}

// 更新服务模型根据服务信息列表
void ServiceTableModel::updateServiceList(const QList<ServiceInfo> &list)
{
    beginResetModel();
    // 重置
    m_svcNameList.clear();
    m_svcInfoList.clear();
    m_nItemCount = 0;
    // 填充数据列表
    for (auto &svc : list) {
        m_svcNameList << svc.getSName();
        m_svcInfoList[svc.getSName()] = svc;
    }
    endResetModel();
}

void ServiceTableModel::updateServiceInfo(const ServiceInfo &info)
{
    // 获取服务名称
    auto sname = info.getSName();
    if (m_svcNameList.contains(sname)) {
        // 更新对应名称的模型索引数据
        for (auto row = 0; row < m_svcNameList.size(); row++) {
            if (m_svcNameList[row] == sname) {
                m_svcInfoList[sname] = info;
                Q_EMIT dataChanged(index(row, 0), index(row, columnCount() - 1));
                break;
            }
        }
    } else {
        // 添加信息到模型中
        auto row = m_svcNameList.size();
        beginInsertRows({}, row, row);
        m_svcNameList << sname;
        m_svcInfoList[sname] = info;
        ++m_nItemCount;
        endInsertRows();
        Q_EMIT itemRowChanged(row);
    }
}

// 模型行数
int ServiceTableModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return m_nItemCount;
}

// 模型列数
int ServiceTableModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return ServiceColumnCount;
}

// 返回表头列信息
QVariant ServiceTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole || role == Qt::AccessibleTextRole) {
        if (headerDataMap.keys().contains(section)) {
            return headerDataMap[section];
        }
        switch (section) {
        case ServiceNameColumn: // 服务名称列头文本
            return tr("Name");
        case ServiceLoadStateColumn: // 服务加载状态列头文本
            return tr("Load");
        case ServiceActiveStateColumn: // 服务活动状态列头文本
            return tr("Active");
        case ServiceSubStateColumn: // 服务子状态列头文本
            return tr("Sub");
        case ServiceStateColumn: // 服务状态列头文本
            return tr("State");
        case ServiceDescriptionColumn: // 服务描述列头文本
            return tr("Description");
        case ServiceMainPIDColumn: // 服务主进程id列头文本
            return tr("PID");
        case ServiceStartupModeColumn: // 服务启动模式列头文本
            return tr("StartupMode");
        default:
            break;
        }
    } else if (role == Qt::TextAlignmentRole) {
        // 默认内容对齐方式
        return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
    } else if (role == Qt::InitialSortOrderRole) {
        // 初始排序方式（降序）
        return QVariant::fromValue(Qt::DescendingOrder);
    }

    return QAbstractTableModel::headerData(section, orientation, role);
}

bool ServiceTableModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role)
{
    if (section == -1) {
        headerDataMap.clear();
        return false;
    } else {
        headerDataMap[section] = value.toString();
        return QAbstractTableModel::setHeaderData(section, orientation, value, role);
    }
}

// 根据模型索引返回数据项内容
QVariant ServiceTableModel::data(const QModelIndex &index, int role) const
{
    auto row = index.row();

    // 检查索引有效性
    if (!index.isValid() || !(row >= 0 && row < m_svcNameList.size())) {
        return QVariant();
    }

    if (role == Qt::DisplayRole || role == Qt::AccessibleTextRole) {
        QString buffer;
        switch (index.column()) {
        case ServiceNameColumn: // 服务名称列头文本
            buffer = m_svcInfoList[m_svcNameList[row]].getSName();
            return QApplication::translate("ServiceTableModel", buffer.toUtf8());
        case ServiceLoadStateColumn: // 服务加载状态列头文本
            buffer = m_svcInfoList[m_svcNameList[row]].getLoadState();
            return QApplication::translate("ServiceTableModel", buffer.toUtf8());
        case ServiceActiveStateColumn: // 服务活动状态列头文本
            buffer = m_svcInfoList[m_svcNameList[row]].getActiveState();
            return QApplication::translate("ServiceTableModel", buffer.toUtf8());
        case ServiceSubStateColumn: // 服务子状态列头文本
            return m_svcInfoList[m_svcNameList[row]].getSubState();
        case ServiceStateColumn: // 服务状态列头文本
            buffer = m_svcInfoList[m_svcNameList[row]].getState();
            return QApplication::translate("ServiceTableModel", buffer.toUtf8());
        case ServiceDescriptionColumn: // 服务描述列头文本
            return m_svcInfoList[m_svcNameList[row]].getDescription();
        case ServiceMainPIDColumn: // 服务主进程id列头文本
        {
            auto pid = m_svcInfoList[m_svcNameList[row]].getMainPID();
            return (pid == 0 ? QVariant(): QVariant(pid));
        }
        case ServiceStartupModeColumn: // 服务启动模式列头文本
            buffer = m_svcInfoList[m_svcNameList[row]].getStartupType();
            return QApplication::translate("ServiceTableModel", buffer.toUtf8());
        default:
            break;
        }
    } else if (role == Qt::TextAlignmentRole) {
        // 返回文本对齐方式
        return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
    }
    return QVariant();
}

// 返回指定索引项的属性
Qt::ItemFlags ServiceTableModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

// 根据父索引获取更多项的信息
void ServiceTableModel::fetchMore(const QModelIndex &parent)
{
    if (parent.isValid())
        return;

    // 至少一次获取100项数据
    int left = m_svcNameList.size() - m_nItemCount;
    int more = qMin(100, left);

    if (more <= 0)
        return;

    beginInsertRows(QModelIndex(), m_nItemCount, m_nItemCount + more - 1);
    m_nItemCount += more;
    endInsertRows();
}

// 根据父索引判断是否有更多项信息可获取 （为动态展示列表项？）
bool ServiceTableModel::canFetchMore(const QModelIndex &parent) const
{
    if (parent.isValid())
        return false;

    return (m_nItemCount < m_svcNameList.size());
}

// 重置模型内容
void ServiceTableModel::reset()
{
    beginRemoveRows({}, 0, m_svcNameList.size() - 1);
    m_svcNameList.clear();
    m_svcInfoList.clear();
    endRemoveRows();
}
