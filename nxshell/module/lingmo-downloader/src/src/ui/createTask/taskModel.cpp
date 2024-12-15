// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @copyright 2020-2021 Uniontech Technology Co., Ltd.
 *
 * @file taskModel.cpp
 *
 * @brief
 *
 * @date 2021-03-23 21:10
 *
 * Author: bulongwei  <bulongwei@uniontech.com>
 *
 * Maintainer: bulongwei  <bulongwei@uniontech.com>
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

#include "taskModel.h"
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

TaskModel::TaskModel(QObject *parent)
    : QAbstractTableModel(parent)
{

}

TaskModel::~TaskModel()
{

}

int TaskModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_linkInfo.size();
}

int TaskModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 6;
}

QVariant TaskModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    int row = index.row();

    LinkInfo data = m_linkInfo.value(row);

    if(role == Qt::AccessibleTextRole){
        switch(index.column()) {
        case TaskModel::Ischecked: {
            return data.isChecked.append("_")+QString::number(row);
        }
        case TaskModel::Name: {
            return data.urlName.append("_")+QString::number(row);
        }
        case TaskModel::Type: {
            return data.type.append("_")+QString::number(row);
        }
        case TaskModel::Size: {
            return data.urlSize.append("_")+QString::number(row);
        }
        case TaskModel::Length: {
            return QString::number(data.length);
        }
        case TaskModel::Url: {
            return data.url.append("_")+QString::number(row);
        }
        case TaskModel::TrueUrl: {
            return data.urlTrueLink.append("_")+QString::number(row);
        }
    }
    }

    switch (role) {
        case TaskModel::Ischecked: {
            return data.isChecked;
        }
        case TaskModel::Name: {
            return data.urlName;
        }
        case TaskModel::Type: {
            return data.type;
        }
        case TaskModel::Size: {
            return data.urlSize;
        }
        case TaskModel::Length: {
            return QString::number(data.length);
        }
        case TaskModel::Url: {
            return data.url;
        }
        case TaskModel::TrueUrl: {
            return data.urlTrueLink;
        }
        default:
            break;
    }
    return QVariant();
}

QVariant TaskModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (role == Qt::BackgroundRole) {
        return QBrush(QColor(Qt::white));
    }

    if (orientation != Qt::Horizontal) {
        return QVariant();
    }

    switch (section) {
    case 0:
        return tr("Name");
    case 1:
        return tr("");
    case 2:
        return tr("Type");
    case 3:
        return tr("Size");
    case 4:
        return "long";
    case 5:
        return "url";
    }

    return QVariant();
}

Qt::ItemFlags TaskModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return QAbstractItemModel::flags(index);
    }
    return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
}

bool TaskModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid()) {
        return false;
    }

    const int row = index.row();

    LinkInfo linkInfo = m_linkInfo.value(row);

    if (index.column() == 0)
        linkInfo.isChecked = value.toString();
    else if (index.column() == 1)
        linkInfo.urlName = value.toString();
    else if (index.column() == 2)
        linkInfo.type = value.toString();
    else if (index.column() == 3)
        linkInfo.urlSize = value.toString();
    else if (index.column() == 4)
        linkInfo.length = value.toLongLong();
    else if (index.column() == 5)
        linkInfo.url = value.toString();
    else if (index.column() == 6)
        linkInfo.urlTrueLink = value.toString();

    m_linkInfo.replace(row, linkInfo);
    emit(dataChanged(index, index));
    return true;
}


bool TaskModel::removeRow(int position, const QModelIndex &index)
{
    Q_UNUSED(index);
    beginRemoveRows(QModelIndex(), position, position);

    m_linkInfo.removeAt(position);

    endRemoveRows();
    return true;
}

bool TaskModel::insertRows(int position, int rows, const QModelIndex &index)
{
    Q_UNUSED(index);
    if(m_linkInfo.size() > position){
        return false;
    }
    beginInsertRows(QModelIndex(), m_linkInfo.size(), m_linkInfo.size());
    m_linkInfo.append(LinkInfo());
    endInsertRows();
    return true;
}


