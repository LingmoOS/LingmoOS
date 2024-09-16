// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @copyright 2020-2021 Uniontech Technology Co., Ltd.
 *
 * @file taskModel.h
 *
 * @brief
 *
 * @date 2021-03-23 21:03
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

#ifndef TABLEVIEWMODEL_H
#define TABLEVIEWMODEL_H

#include <QAbstractTableModel>
#include "analysisurl.h"

/**
 * @class TaskModel
 * @brief 表格数据管理模块
 */
class TaskModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    TaskModel(QObject *parent = nullptr);
    ~TaskModel();

    /**
     * @brief 数据角色
     */
    enum DataRole {
        Ischecked = 0,
        Name,
        Type,
        Size,
        Length,
        Url,
        TrueUrl
    };

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
   // QVariant getData(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    bool removeRow(int position, const QModelIndex &index = QModelIndex());
    bool insertRows(int position, int rows, const QModelIndex &index = QModelIndex()) override;

private:

signals:


private slots:

private:
    QList<LinkInfo> m_linkInfo;
};

#endif // TABLEVIEWMODEL_H
