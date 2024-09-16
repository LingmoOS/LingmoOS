// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @copyright 2020-2020 Uniontech Technology Co., Ltd.
 *
 * @file deleteitemthread.h
 *
 * @brief 删除任务类，是一个独立线程
 *
 * @date 2020-06-09 10:01
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

#ifndef DELETEITEMTHREAD_H
#define DELETEITEMTHREAD_H

#include "global.h"
#include <QThread>
#include "tableView.h"
#include "aria2rpcinterface.h"

using namespace Global;

/**
 * @class DeleteItemThread
 * @brief 删除任务类，是一个独立线程
*/
class DeleteItemThread : public QThread
{
    Q_OBJECT
public:
    DeleteItemThread();
    DeleteItemThread(QList<DeleteDataItem *> &deleteList, TableView *pRecycleTableview, bool m_IfDeleteLocal, QString m_StrDeleteType);
    DeleteItemThread(QList<DownloadDataItem *> &deleteList, TableView *pDownloadingTableview, bool m_IfDeleteLocal, QString m_StrDeleteType);
    void run();

private:
    /**
     * @brief 删除回收站数据
    */
    void deleteRecycleData();
    /**
      * @brief 删除下载列表数据
     */
    void deleteDownloadData();

    /**
      * @brief 删除目录
      * @return
      */
    bool deleteDirectory(const QString &path);

    // bool checkPath(DownloadDataItem &item);
signals:
    /**
      * @brief 通知主界面调用aria2删除任务
     */
    void Aria2Remove(QString gId, QString id);

    /**
      * @brief 删除完成
     */
    void removeFinished();

private:
    QList<DeleteDataItem*> m_RecycleDeleteList;
    TableView *m_RecycleTableview;
    bool m_IfDeleteLocal;
    QString m_StrDeleteType;
    QList<DownloadDataItem*> m_DeleteList;
    TableView *m_DownloadingTableview;
};

#endif // DELETEITEMTHREAD_H
