// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @copyright 2020-2020 Uniontech Technology Co., Ltd.
 *
 * @file tableModel.cpp
 *
 * @brief 表格数据管理模块
 *
 * @date 2020-06-09 09:58
 *
 * Author: zhaoyue  <zhaoyue@uniontech.com>
 *
 * Maintainer: zhaoyue  <zhaoyue@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "tableModel.h"
#include <QDebug>
#include <QString>
#include <QStandardItemModel>
#include <QMimeData>

#include "settings.h"
#include <dpinyin.h>
#include "global.h"
#include "func.h"
using namespace Global;
using namespace DTK_CORE_NAMESPACE;

TableModel::TableModel(int Flag, QObject *parent)
    : QAbstractTableModel(parent)
    , m_Mode(Downloading)
    , m_TableviewtabFlag(Flag)
    , m_DownloadingCount(0)
{
    connect(this, &TableModel::checkDatachange, this, &TableModel::onCheckdatachange);
    m_SortColumn = 0;
    m_SortOrder = Qt::AscendingOrder;
}

TableModel::~TableModel()
{
    for(auto data : m_DataList) {
        delete data;
    }
    for(auto data : m_RecyleList) {
        delete data;
    }
}

void TableModel::onCheckdatachange(int flag)
{
    int checkNum = 0;
    QList<DownloadDataItem *> activeList;
    QList<DownloadDataItem *> finishList;

    if (flag == 0) {
        for (DownloadDataItem *item : m_DataList) {
            if (m_Mode == Downloading) {
                if (item->status != Global::DownloadTaskStatus::Complete) {
                    activeList.append(item);
                }
            } else {
                if (item->status == Global::DownloadTaskStatus::Complete) {
                    finishList.append(item);
                }
            }
        }
        if ((m_Mode == Downloading) && (activeList.size() > 0)) {
            for (DownloadDataItem *item : activeList) {
                if (item->isChecked) {
                    checkNum++;
                }
            }
            if (checkNum == activeList.size()) {
                emit tableviewAllcheckedOrAllunchecked(true);
            } else {
                emit tableviewAllcheckedOrAllunchecked(false);
            }
        }
        if ((m_Mode == Finished) && (finishList.size() > 0)) {
            for (DownloadDataItem *item : finishList) {
                if (item->isChecked) {
                    checkNum++;
                }
            }
            if (checkNum == finishList.size()) {
                emit tableviewAllcheckedOrAllunchecked(true);
            } else {
                emit tableviewAllcheckedOrAllunchecked(false);
            }
        }
    } else {
        for (int i = 0; i < m_RecyleList.size(); i++) {
            DeleteDataItem *del_data = m_RecyleList.at(i);
            if (del_data->isChecked) {
                checkNum++;
            }
        }
        if (checkNum == m_RecyleList.size()) {
            emit tableviewAllcheckedOrAllunchecked(true);
        } else {
            emit tableviewAllcheckedOrAllunchecked(false);
        }
    }
}

DownloadDataItem *TableModel::find(const QString &taskId)
{
    if (m_Map.contains(taskId)) {
        return m_Map.value(taskId);
    }
    return nullptr;
}

DeleteDataItem *TableModel::find(const QString &gid, int flag)
{
    Q_UNUSED(flag);
    if (m_Deletemap.contains(gid)) {
        return m_Deletemap.value(gid);
    }
    return nullptr;
}

bool TableModel::append(DownloadDataItem *data)
{
    if (data == nullptr) {
        return false;
    }
    const int row = m_DataList.size();

    beginInsertRows(QModelIndex(), row, row);
    m_DataList.append(data);
    m_Map.insert(data->taskId, data);
    endInsertRows();
    return true;
}

bool TableModel::append(DeleteDataItem *data)
{
    if (data == nullptr) {
        return false;
    }
    const int row = m_RecyleList.size();

    beginInsertRows(QModelIndex(), row, row);
    m_RecyleList.append(data);
    m_Deletemap.insert(data->taskId, data);
    endInsertRows();
    return true;
}

bool TableModel::removeItem(DownloadDataItem *data)
{
    if (data == nullptr) {
        return false;
    }
    if (m_Map.contains(data->taskId)) {
        beginRemoveRows(QModelIndex(), m_DataList.indexOf(data), m_DataList.indexOf(data));
        m_Map.remove(data->taskId);
        m_DataList.removeOne(data);
        m_RenderList.removeOne(data);
        delete data;
        data = nullptr;
        endRemoveRows();
    }
    return true;
}

bool TableModel::removeItem(DeleteDataItem *data)
{
    if (data == nullptr) {
        return false;
    }
    if (m_Deletemap.contains(data->taskId)) {
        beginRemoveRows(QModelIndex(), m_RecyleList.indexOf(data), m_RecyleList.indexOf(data));
        m_Deletemap.remove(data->taskId);
        m_RecyleList.removeOne(data);
        delete data;
        data = nullptr;
        endRemoveRows();
    }
    return true;
}

bool TableModel::removeItems()
{
    beginRemoveRows(QModelIndex(), 0, m_DataList.size());
    qDeleteAll(m_DataList.begin(), m_DataList.end());
    m_DataList.clear();
    m_Map.clear();
    endRemoveRows();
    return true;
}

bool TableModel::removeRecycleItems()
{
    beginRemoveRows(QModelIndex(), 0, m_RecyleList.size());
    qDeleteAll(m_RecyleList.begin(), m_RecyleList.end());
    m_RecyleList.clear();
    m_Deletemap.clear();
    endRemoveRows();
    return true;
}

bool TableModel::switchDownloadingMode()
{
    m_Mode = Downloading;
    m_RenderList.clear();

    for (DownloadDataItem *item : m_DataList) {
        if ((item->status == Global::DownloadTaskStatus::Active) || (item->status == Global::DownloadTaskStatus::Paused) || (item->status == Global::DownloadTaskStatus::Waiting) || (item->status == Global::DownloadTaskStatus::Lastincomplete) || (item->status == Global::DownloadTaskStatus::Error)) {
            m_RenderList.append(item);
        }
    }
    sortDownload(m_SortColumn, m_SortOrder);
    return true;
}

bool TableModel::switchFinishedMode()
{
    m_Mode = Finished;
    m_RenderList.clear();

    for (DownloadDataItem *item : m_DataList) {
        if (item->status == Global::DownloadTaskStatus::Complete) {
            m_RenderList.append(item);
        }
    }
    sortDownload(m_SortColumn, m_SortOrder);
    return true;
}

int TableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    if (m_TableviewtabFlag == 0) {
        return m_RenderList.size();
    } else {
        return m_RecyleList.size();
    }
}

int TableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    return 5;
}

QVariant TableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }
    const int row = index.row();
    const DownloadDataItem *data = nullptr;
    const DeleteDataItem *deldata = nullptr;
    QChar sizeSepChar;

    if (m_TableviewtabFlag == 0) {
        if ((m_RenderList.size() > 0) && (m_RenderList.size() > row)) {
            data = m_RenderList.at(row);
            sizeSepChar = (!data->totalLength.isEmpty()) ? '/' : ' ';
        } else {
            return QVariant();
        }
    } else {
        deldata = m_RecyleList.at(row);
    }

    QString fileName;
    QString savePath;
    QString gid;
    QString url;
    QString deleteTime;
    switch (role) {

    case Qt::AccessibleTextRole:
    case Qt::AccessibleDescriptionRole: {
        switch (index.column()) {
        case 0:
            if (m_TableviewtabFlag == 0) {
                return data->isChecked ? "true" : "false";
            } else {
                return deldata->isChecked ? "true" : "false";
            }
        case 1:
            if (m_TableviewtabFlag == 0) {
                return data->fileName;
            } else {
                return deldata->fileName;
            }
        case 2:
            if (m_TableviewtabFlag == 0) {
                return data->totalLength;
            } else {
                return deldata->totalLength;
            }
        case 3:
            if (m_TableviewtabFlag == 0) {
                switch (data->status) {
                    case 0:
                    return "Active";
                    case 1:
                    return "Waiting";
                    case 2:
                    return "Paused";
                    case 3:
                    return "Complete";
                    case 4:
                    return "Removed";
                    case 5:
                    return "Lastincomplete";
                    case 6:
                    return "Error";
                }
            } else {
                switch (deldata->status) {
                    case 0:
                    return "Active";
                    case 1:
                    return "Waiting";
                    case 2:
                    return "Paused";
                    case 3:
                    return "Complete";
                    case 4:
                    return "Removed";
                    case 5:
                    return "Lastincomplete";
                    case 6:
                    return "Error";
                }
            }
        case 4:
            if (m_TableviewtabFlag == 0) {
                return data->time;
            } else {
                return deldata->deleteTime;
            }
        }
    }

    case TableModel::Ischecked: {
        if (m_TableviewtabFlag == 0) {
            return data->isChecked ? Qt::Checked : Qt::Unchecked;
        } else {
            return deldata->isChecked ? Qt::Checked : Qt::Unchecked;
        }
    }

    case TableModel::FileName: {
        if (m_TableviewtabFlag == 1) {
            return deldata->fileName;
        }
        if (m_TableviewtabFlag == 0) {
            return data->fileName;
        }
        break;
    }

    case TableModel::Size: {
        if (m_TableviewtabFlag == 0) {
            // return QString("%1%2%3
            //  %4").arg(data->completedLength).arg(sizeSepChar).arg(data->totalLength).arg(data->percent);
            if (m_Mode == Downloading) {
                return QString("%1%2%3 ").arg(data->completedLength).arg(sizeSepChar).arg(data->totalLength);
            } else {
                return QString("%1 ").arg(data->totalLength);
            }

        } else {
            return deldata->totalLength;
        }
    }
    case TableModel::Speed: {
        if (m_TableviewtabFlag == 0) {
            return data->speed; // (data->status != Global::DownloadJobStatus::Paused) ? data->speed : "0KB/s";
        }
        break;
    }

    case TableModel::Time: {
        if (m_TableviewtabFlag == 0) {
            return data->time;
        } else {
            return deldata->deleteTime;
        }
    }

    case TableModel::Status: {
        if (m_TableviewtabFlag == 0) {
            return data->status;
        } else {
            return deldata->status;
        }
    }

    case TableModel::GID: {
        if (m_TableviewtabFlag == 0) {
            return data->gid;
        } else {
            return deldata->gid;
        }
    }

    case TableModel::taskId: {
        if (m_TableviewtabFlag == 0) {
            return data->taskId;
        } else {
            return deldata->taskId;
        }
    }
    case TableModel::createTime: {
        if (m_TableviewtabFlag == 0) {
            return data->createTime;
        } else {
            return deldata->deleteTime;
        }
    }

    case TableModel::Percent: {
        if (m_TableviewtabFlag == 0) {
            return data->percent;
        }
        break;
    }

    case TableModel::TotalLength: {
        if (m_TableviewtabFlag == 0) {
            return data->totalLength;
        } else if (m_TableviewtabFlag == 1) {
            return deldata->totalLength;
        }
        break;
    }

    case TableModel::SavePath: {
        if (m_TableviewtabFlag == 0) {
            return data->savePath;
        } else {
            return deldata->savePath;
        }
    }
    case TableModel::IsHide: {
        if (m_TableviewtabFlag == 0) {
            return data->isHide;
        } else {
            return deldata->isHide;
        }
    }
    case TableModel::connection: {
        if (m_TableviewtabFlag == 0) {
            return data->connection;
        }
        break;
    }
    case TableModel::announceList: {
        if (m_TableviewtabFlag == 0) {
            return data->announceList;
        }
        break;
    }
    }
    return QVariant();
}

QVariant TableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole) {
        return QVariant();
    }

    if (role == Qt::BackgroundRole) {
        return QBrush(QColor(Qt::white));
    }

    if (orientation != Qt::Horizontal) {
        return QVariant();
    }

    switch (section) {
    case 0:
        return tr("");

    case 1:
        return tr("Name");

    case 2:
        return tr("Size");

    case 3:
        if ((m_Mode == Downloading) && (m_TableviewtabFlag == 0)) {
            return tr("Status");
        } else if ((m_Mode == Finished) && (m_TableviewtabFlag == 0)) {
            return tr("Time finished");
        } else {
            return tr("Time deleted");
        }

    case 4:
        if ((m_Mode == Downloading) && (m_TableviewtabFlag == 0)) {
            return tr("Status");
        } else if ((m_Mode == Finished) && (m_TableviewtabFlag == 0)) {
            return tr("Time finished");
        } else {
            return tr("Time deleted");
        }
    }

    return QVariant();
}

Qt::ItemFlags TableModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return QAbstractItemModel::flags(index);
    }
    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable; //ItemIsEnabled,表明这一项可以使用，ItemIsSelectable 表明这一项可以选中。

    if (index.column() == 1) //增加第二列的控制选项。
        flags |= Qt::ItemIsUserCheckable | Qt::ItemIsEditable | Qt::ItemIsDragEnabled;
    return flags;
}

//bool TableModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
//{
//    QMimeData *data = new QMimeData;
//}

bool TableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid()) {
        return false;
    }

    int nColumn = index.column();
    const int row = index.row();

    DownloadDataItem *data = nullptr;
    DeleteDataItem *deldata = nullptr;

    if (m_TableviewtabFlag == 0) {
        data = m_RenderList.at(row);
    } else {
        deldata = m_RecyleList.at(row);
    }

    switch (role) {
    case TableModel::Ischecked: {
        if (nColumn == 0) {
            if (m_TableviewtabFlag == 0) {
                data->isChecked = value.toBool();
                m_RenderList.replace(index.row(), data);
            } else {
                deldata->isChecked = value.toBool();
                m_RecyleList.replace(index.row(), deldata);
            }

            // emit dataChanged(index,index);
            emit checkDatachange(m_TableviewtabFlag);
            emit CheckChange(value.toBool(), m_TableviewtabFlag);
            return true;
        }
        break;
    }
    case TableModel::FileName: {
        if (nColumn == 1) {
            if (m_TableviewtabFlag == 0) {
                data->fileName = value.toString();
                m_RenderList.replace(index.row(), data);
            } else {
                deldata->fileName = value.toString();
                m_RecyleList.replace(index.row(), deldata);
            }
            return true;
        }
        break;
    }
    case TableModel::SavePath: {
        if (m_TableviewtabFlag == 0) {
            data->savePath = value.toString();
            m_RenderList.replace(index.row(), data);
        } else {
            deldata->savePath = value.toString();
            m_RecyleList.replace(index.row(), deldata);
        }
        break;
    }

    default:
        return false;
    }
    return false;
}

int TableModel::getTablemodelMode()
{
    return m_Mode;
}

const QList<DownloadDataItem *> &TableModel::dataList()
{
    return m_DataList;
}

const QList<DownloadDataItem *> &TableModel::renderList()
{
    return m_RenderList;
}

const QList<DeleteDataItem *> &TableModel::recyleList()
{
    return m_RecyleList;
}

QMap<QString, DownloadDataItem *> &TableModel::getTableModelMap()
{
    return m_Map;
}

int TableModel::DownloadingCount()
{
    if (Downloading == m_Mode) {
        return m_RenderList.count();
    } else {
        return m_DataList.count() - m_RenderList.count();
    }
}

typedef bool (*LessThan)(const QPair<QVariant, int> &left,
                         const QPair<QVariant, int> &right);

bool itemLessThan(const QPair<QVariant, int> &left,
                  const QPair<QVariant, int> &right)
{
    return left.first < right.first;
}

bool itemGreaterThan(const QPair<QVariant, int> &left,
                     const QPair<QVariant, int> &right)
{
    return right.first < left.first;
}

void TableModel::sort(int column, Qt::SortOrder order)
{
    if (0 == column) {
        return;
    }
    if (0 == m_TableviewtabFlag) {
        m_SortColumn = column;
        m_SortOrder = order;
        sortDownload(column, order);
    } else if (1 == m_TableviewtabFlag) {
        sortRecycle(column, order);
    }
}

void TableModel::sortDownload(int column, Qt::SortOrder order)
{
    if (Finished == m_Mode && column == 3) {
        return;
    }
    QVector<QPair<QVariant, int>> sortable;
    QVector<int> unsortable;
    int role = 0;
    switch (column) {
    case 0:
        role = TableModel::createTime;
        break;
    case 1:
        role = TableModel::FileName;
        break;
    case 2:
        if (Downloading == m_Mode) {
            role = TableModel::TotalLength;
        } else {
            role = TableModel::TotalLength;
        }
        break;
    case 3:
        if (Downloading == m_Mode) {
            role = TableModel::Status;
        }
        break;
    case 4:
        if (Finished == m_Mode) {
            role = TableModel::Time;
        }
        break;
    case 5:
        if (Downloading == m_Mode) {
            role = TableModel::Speed;
        }
        break;
    case 6:
        if (Downloading == m_Mode) {
            role = TableModel::createTime;
        }
        break;
    }
    double num = -1;
    int hideCount = 0;
    for (int row = 0; row < rowCount(); ++row) {
        QVariant itm = DTK_NAMESPACE::Core::Chinese2Pinyin(data(index(row, 0), role).toString());
        if (role == TableModel::Size || role == TableModel::TotalLength) {
            num = Func::formatFileSize(itm.toString());
            sortable.append(QPair<QVariant, int>(num, row));
        } else if (role == TableModel::Speed) {
            num = Func::formatSpeed(itm.toString());
            sortable.append(QPair<QVariant, int>(num, row));
        } else {
            if (!itm.isNull()) {
                sortable.append(QPair<QVariant, int>(itm, row));
            } else {
                unsortable.append(row);
            }
        }
    }
    if (column == 5) {
        LessThan compare = &itemGreaterThan;
        std::stable_sort(sortable.begin(), sortable.end(), compare);
    } else {
        LessThan compare = (order == Qt::AscendingOrder ? &itemLessThan : &itemGreaterThan);
        std::stable_sort(sortable.begin(), sortable.end(), compare);
    }

    QList<DownloadDataItem *> sortData;
    emit layoutAboutToBeChanged();
    int nSwapCount = rowCount() - hideCount;
    for (int i = 0; i < nSwapCount; i++) {
        int r = (i < sortable.count()
                     ? sortable.at(i).second
                     : unsortable.at(i - sortable.count()));
        sortData.append(m_RenderList[r]);
    }
    m_RenderList = sortData;
    emit layoutChanged();
}

void TableModel::sortRecycle(int column, Qt::SortOrder order)
{
    QVector<QPair<QVariant, int>> sortable;
    QVector<int> unsortable;

    sortable.reserve(rowCount());
    unsortable.reserve(rowCount());

    int role = 0;
    switch (column) {
    case 1:
        role = TableModel::FileName;
        break;
    case 2:
        role = TableModel::TotalLength;
        break;
    case 4:
        role = TableModel::Time;
        break;
    }
    double num = -1;
    int hideCount = 0;
    for (int row = 0; row < rowCount(); ++row) {
        QVariant itm = DTK_NAMESPACE::Core::Chinese2Pinyin(data(index(row, column), role).toString());
        if (role == TableModel::TotalLength) {
            num = Func::formatFileSize(itm.toString());
            sortable.append(QPair<QVariant, int>(num, row));
        } else {
            if (!itm.isNull()) {
                sortable.append(QPair<QVariant, int>(itm, row));
            } else {
                unsortable.append(row);
            }
        }
    }
    LessThan compare = (order == Qt::AscendingOrder ? &itemLessThan : &itemGreaterThan);
    std::stable_sort(sortable.begin(), sortable.end(), compare);

    QList<DeleteDataItem *> sortData;
    emit layoutAboutToBeChanged();
    int nSwapCount = rowCount() - hideCount;
    for (int i = 0; i < nSwapCount; i++) {
        int r = (i < sortable.count()
                     ? sortable.at(i).second
                     : unsortable.at(i - sortable.count()));
        sortData.append(m_RecyleList[r]);
    }
    m_RecyleList = sortData;
    emit layoutChanged();
}
