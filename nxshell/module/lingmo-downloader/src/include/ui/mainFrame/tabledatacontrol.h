// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @copyright 2020-2020 Uniontech Technology Co., Ltd.
 *
 * @file tabledatacontrol.h
 *
 * @brief 表格数据管理类
 *
 * @date 2020-06-10 17:55
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

#ifndef TABLEDATACONTROL_H
#define TABLEDATACONTROL_H

#include <QObject>
#include <QThread>

class TableView;
struct TaskInfo;
namespace Global {
struct DownloadDataItem;
struct DeleteDataItem;
} // namespace Global

/**
 * @class tableDataControl
 * @brief 列表数据管理，包括一些新建、删除等
 */
class TableDataControl : public QObject
{
    Q_OBJECT
public:
    explicit TableDataControl(TableView *pTableView, QObject *parent = nullptr);

    void initTabledata();

    /**
     * @brief 设置RecycleTable
     */
    bool setRecycleTable(TableView *pRecycleTable);

    /**
     * @brief 将正在下载列表里文件不存在的任务移到回收站
     */
    bool removeDownloadListJob(Global::DownloadDataItem *pData,
                               bool isDeleteAria2 = true, bool isAddToRecycle = true);

    /**
     * @brief aria2下载事件
     */
    bool aria2MethodAdd(QJsonObject &json, QString &searchContent);

    /**
     * @brief aria2状态改变事件
     */
    bool aria2MethodStatusChanged(QJsonObject &json, int iCurrentRow, QString &searchContent);

    /**
     * @brief aria2关闭事件
     */
    bool aria2MethodShutdown(QJsonObject &json);

    /**
     * @brief aria2获取文件事件
     */
    bool aria2MethodGetFiles(QJsonObject &json, int iCurrentRow);

    /**
     * @brief aria2继续下载事件
     */
    bool aria2MethodUnpause(QJsonObject &json, int iCurrentRow);

    /**
     * @brief aria2继续所有下载事件
     */
    bool aria2MethodUnpauseAll(QJsonObject &json, int iCurrentRow);

    /**
     * @brief aria2获取全局状态
     */
    bool aria2GetGlobalStatus(QJsonObject &json);

    /**
     * @brief aria2强制删除事件
     */
    bool aria2MethodForceRemove(QJsonObject &json);

    /**
     * @brief 退出之前保存
     */
    bool saveDataBeforeClose();

    /**
     * @brief 刷新数据库
     */
    bool updateDb();

    /**
     * @brief 查找的文本改变
    */
    bool searchEditTextChanged(QString text, QList<QString> &taskIDList,
                            QList<int> &taskStatusList,QList<QString> &tasknameList);

    /**
     * @brief 删除ACtion槽函数
    */
    bool onDelAction(int currentTab);

    /**
     * @brief  获取删除正在下载和已完成列表窗口确定信号
     * @param ischecked 是否删除本地文件，true 删除本地文件；false 不删除
     * @param permanent 是否彻底删除，true彻底删除；false不彻底删除
     */
    bool onDeleteDownloadListConfirm(bool ischecked, bool permanent, TableView *pRecycleTableView);

    /**
     * @brief  获取删除回收站列表窗口确定信号
     * @param ischecked 是否删除本地文件，true 删除本地文件；false 不删除
     * @param permanent 是否彻底删除，true彻底删除；false不彻底删除
     */
    bool onDeleteRecycleListConfirm(bool ischecked, bool permanent);

    /**
     * @brief  正在下载列表和已完成列表重新下载
     */
    bool downloadListRedownload(QString id);

    /**
     * @brief  已删除列表重新下载
     */
    bool recycleListRedownload(QString id);

    /**
     * @brief 清空共享内存
     */
    void clearShardMemary();

private:
    /**
     * @brief 获取url中的文件名称
     * @param url 下载地址
     */
    QString getFileName(const QString &url);

    /**
     * @brief 处理设置界面通知设置函数
     */
    void dealNotificaitonSettings(QString statusStr, QString fileName, QString errorCode);

    /**
     * @brief 格式化文件大小 （1B1KB1MB1GB）
     */
    QString formatFileSize(long size);

    /**
     * @brief 从配置文件中获取下载路径
     */
    QString getDownloadSavepathFromConfig();

    /**
     * @brief 格式化下载速度（1B1KB1MB1GB  /S）
     */
    QString formatDownloadSpeed(long size);

    /**
     * @brief 检查任务状态
     * @return true: 有激活的任务  false：没有激活的任务
     */
    bool checkTaskStatus();

    /**
     * @brief 删除任务
     * @return true: 删除成功  false：删除失败
     */
    bool deleteTask(bool ifDeleteLocal, TableView *pRecycleTableView);

    /**
     * @brief 重新下载任务
     * @return true: 重新下载成功  false：失败
     */
    bool reDownloadTask(QString taskId, QString filePath, QString fileName, QString url);


    /**
     * @brief 下载的任务，文件不存在时的处理逻辑
     * @return true: 重新下载成功  false：失败
     */
    bool excuteFileNotExist(Global::DownloadDataItem *data, QString filename, QString taskId);
public slots:

    /**
     * @brief 异常处理确认
     */
    void onUnusualConfirm(int index, const QString &taskIds);

signals:
    /**
     * @brief 文件为bt，自动开始下载
     */
    void AutoDownloadBt(QString btFilePath);

    /**
     * @brief 文件为metalink，自动开始下载
     */
    void AutoDownloadMetalink(QString filePath);

    /**
     * @brief 重新下载信号
     */
    void RedownloadJob(QString taskId, int rd);

    /**
     * @brief http任务源文件被删除异常下载的信号
     */
    void DownloadUnusuaHttpJob(QString urlList, QString savePath, QString filename = "", QString type = "", QString leng = "");

    /**
     * @brief bt任务源文件被删除异常下载的信号
     */
    void DownloadUnusuaBtJob(QString btPath, QMap<QString, QVariant> &opt, QString infoName, QString infoHash);

    /**
     * @brief 删除完成
     */
    void removeFinished();

    /**
     * @brief 受到删除bt的回复，开始下载任务
     */
    void startDownload();

    /**
     * @brief 进行下载完成后的操作
     */
    void whenDownloadFinish();

    /**
     * @brief 增加一个最大下载任务数
     */
    void addMaxDownloadTask(int num);

    /**
     * @brief 继续下载任务信号
     */
    void unPauseTask(Global::DownloadDataItem *pItem);

private:
    TableView *m_DownloadTableView;
    TableView *m_RececleTableView;
    QList<Global::DownloadDataItem *> m_DeleteList;
    QList<Global::DeleteDataItem *> m_RecycleDeleteList;
};
#endif // TABLEDATACONTROL_H
