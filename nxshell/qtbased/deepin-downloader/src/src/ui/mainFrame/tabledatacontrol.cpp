// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @copyright 2020-2020 Uniontech Technology Co., Ltd.
 *
 * @file tabledatacontrol.cpp
 *
 * @brief 表格数据管理类
 *
 * @date 2020-06-10 17:56
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
#include "tabledatacontrol.h"
#include <unistd.h>
#include <QDebug>
#include <QScrollBar>
#include <QMouseEvent>
#include <QHeaderView>
#include <QModelIndex>
#include <QJsonObject>
#include <QDateTime>
#include <QProcess>
#include <QThread>
#include <QDesktopServices>
#include <QUuid>
#include <QDBusInterface>
#include <QSharedMemory>
#include <QBuffer>
#include <QMimeDatabase>
#include <QTimer>
#include <QJsonArray>
#include <dpinyin.h>
#include <iostream>
#include <memory>

#include "../database/dbinstance.h"
#include "global.h"
#include "../aria2/aria2rpcinterface.h"
#include "tableModel.h"
#include "headerView.h"
#include "itemDelegate.h"
#include "settings.h"
#include "topButton.h"
#include "tableView.h"
#include "deleteitemthread.h"
#include "messagebox.h"
#include "func.h"
using namespace Global;
TableDataControl::TableDataControl(TableView *pTableView, QObject *parent)
    : QObject(parent)
    , m_DownloadTableView(pTableView)
{
}

bool TableDataControl::setRecycleTable(TableView *pRecycleTable)
{
    if (pRecycleTable == nullptr) {
        return false;
    }
    m_RececleTableView = pRecycleTable;
    return true;
}

bool TableDataControl::removeDownloadListJob(Global::DownloadDataItem *pData,
                                             bool isDeleteAria2, bool isAddToRecycle)
{
    if (pData == nullptr) {
        return false;
    }
    QFileInfo fileinfo(pData->savePath);
    if (fileinfo.isDir() && pData->savePath.contains(pData->fileName) && !pData->fileName.isEmpty()) {
        QDir tar(pData->savePath);
        tar.removeRecursively();
        if (isDeleteAria2) {
            QString ariaTempFile = pData->savePath + ".aria2";
            QTimer::singleShot(3000, [=]() {
                QFile::remove(ariaTempFile);
            });
        }

    } else {
        if (!pData->savePath.isEmpty()) {
            QFile::remove(pData->savePath);
            if (isDeleteAria2) {
                QString ariaTempFile = pData->savePath + ".aria2";
                QTimer::singleShot(3000, [=]() {
                    QFile::remove(ariaTempFile);
                });
            }
        }
    }
    if (isAddToRecycle) {
        DeleteDataItem *delData = new DeleteDataItem;
        delData->taskId = pData->taskId;
        delData->gid = pData->gid;
        delData->url = pData->url;
        delData->fileName = pData->fileName;
        delData->savePath = pData->savePath;
        delData->isChecked = false;
        delData->status = Global::DownloadTaskStatus::Removed;
        delData->totalLength = pData->totalLength;
        delData->completedLength = pData->completedLength;
        delData->deleteTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
        delData->finishTime = pData->time;
        m_RececleTableView->getTableModel()->append(delData);
        TaskStatus getStatus;
        TaskStatus downloadStatus(pData->taskId,
                                  Global::DownloadTaskStatus::Removed,
                                  QDateTime::currentDateTime(),
                                  pData->completedLength,
                                  pData->speed,
                                  pData->totalLength,
                                  pData->percent,
                                  pData->total,
                                  QDateTime::fromString("", "yyyy-MM-dd hh:mm:ss"));
        if (DBInstance::getTaskStatusById(pData->taskId, getStatus)) {
            DBInstance::updateTaskStatusById(downloadStatus);
        } else {
            DBInstance::addTaskStatus(downloadStatus);
        }
    } else {
        DBInstance::delTask(pData->taskId);
    }
    m_DownloadTableView->getTableModel()->removeItem(pData);
    return true;
}

bool TableDataControl::aria2MethodAdd(QJsonObject &json, QString &searchContent)
{
    QString id = json.value("id").toString();
    if (id == "dht.dat" || id == "dht6.dat") {
        return false;
    }
    QString gId = json.value("result").toString();
    if (json.value("result").isArray()) {
        gId = json.value("result").toArray().at(0).toString();
    }
    DownloadDataItem *findData = m_DownloadTableView->getTableModel()->find(id);
        if (findData != nullptr) {
            findData->gid = gId;
            findData->taskId = id;
            QDateTime finishTime = QDateTime::fromString("", "yyyy-MM-dd hh:mm:ss");
            TaskStatus downloadStatus(findData->taskId,
                                      Global::DownloadTaskStatus::Active,
                                      QDateTime::currentDateTime(),
                                      findData->completedLength,
                                      findData->speed,
                                      findData->totalLength,
                                      findData->percent,
                                      findData->total,
                                      finishTime);
            TaskStatus task;
            DBInstance::getTaskStatusById(findData->taskId, task);
            if (!task.taskId.isEmpty()) {
                DBInstance::updateTaskStatusById(downloadStatus);
            } else {
                DBInstance::addTaskStatus(downloadStatus);
            }
            findData->status = Global::DownloadTaskStatus::Active;
        } else {
            // 获取下载信息
            // aria2c->tellStatus(gId, gId);
            Aria2RPCInterface::instance()->getFiles(gId, id);
            //std::shared_ptr<DownloadDataItem> data(new DownloadDataItem);
            //QSharedPointer<DownloadDataItem> data = QSharedPointer<DownloadDataItem>(new DownloadDataItem);
            DownloadDataItem* data = new DownloadDataItem;
            data->taskId = id;
            data->gid = gId;
            data->isChecked = false;
            data->completedLength = "0KB";
            QDateTime time = QDateTime::currentDateTime();
            data->createTime = time.toString("yyyy-MM-dd hh:mm:ss");
            TaskInfo taskInfo;
            DBInstance::getTaskByID(id, taskInfo);
            TaskInfo task;
            if (!taskInfo.taskId.isEmpty()) {
                task = TaskInfo(taskInfo.taskId,
                                gId,
                                0,
                                taskInfo.url,
                                taskInfo.downloadPath,
                                taskInfo.downloadFilename,
                                time);
                DBInstance::updateTaskInfoByID(task);
            } else {
                task = TaskInfo(id, gId, 0, "", "", "Unknown", time);
                DBInstance::addTask(task);
            }
            data->fileName = taskInfo.downloadFilename;
            data->savePath = taskInfo.downloadPath; // + "/" + getTaskInfo.m_downloadFilename;
            data->url = taskInfo.url;
            data->totalLength = taskInfo.fileLength;
            m_DownloadTableView->getTableModel()->append(data);
            m_DownloadTableView->getTableHeader()->onHeaderChecked(false);
            if ((!searchContent.isEmpty()) && !data->fileName.contains(searchContent)) {
                TableModel *dtModel = m_DownloadTableView->getTableModel();
                m_DownloadTableView->setRowHidden(dtModel->rowCount(), true);
            }
            qDebug() << "aria2MethodAdd: " << /*taskInfo.url << */"    " << QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
        }
        return true;
}

bool TableDataControl::aria2MethodStatusChanged(QJsonObject &json, int iCurrentRow, QString &searchContent)
{
    QJsonObject result = json.value("result").toObject();
    QJsonObject bittorrent = result.value("bittorrent").toObject();
    QString filePath;
    QString taskId = json.value("id").toString();
    QJsonArray files = result.value("files").toArray();
    if (files.size() == 1) {
        filePath = files[0].toObject().value("path").toString();
    } else {
        QString path = files[0].toObject().value("path").toString();
        QString path2 = result.value("dir").toString();
        filePath = path2 + "/" + path.split('/').at(path2.split('/').count());
        //filePath = path.left(path.count() - path.split('/').last().count() - 1);
    }

    QString fileUri = files[0].toObject().value("uris").toArray()[0].toObject().value("uri").toString();
    QString gId = result.value("gid").toString();
    long totalLength = result.value("totalLength").toString().toLong(); //字节
    long completedLength = result.value("completedLength").toString().toLong(); //字节
    long downloadSpeed = result.value("downloadSpeed").toString().toLong(); //字节/每秒
    QString fileName = getFileName(filePath);
    QString statusStr = result.value("status").toString();
    QString errorCode = result.value("errorCode").toString();
    int percent = 0;
    int status = 0;
    if (filePath.startsWith("[METADATA]")) {
        QString dir = result.value("dir").toString();
        QString infoHash = result.value("infoHash").toString();
        filePath = dir + "/" + infoHash + ".torrent";
    }
    if ((completedLength != 0) && (totalLength != 0)) {
        double tempPercent = completedLength * 100.0 / totalLength;
        percent = static_cast<int>(tempPercent);
        if ((percent < 0) || (percent > 100)) {
            percent = 0;
        }
        if (completedLength == totalLength) {
            statusStr = "complete";
        }
    }
    DownloadDataItem *data = m_DownloadTableView->getTableModel()->find(taskId);
    if (data == nullptr) {
        return false;
    }
    data->connection = result.value("connections").toString().toLong();
    data->announceList = bittorrent.value("announceList").toArray().size();
    if (statusStr == "active") {
        status = Global::DownloadTaskStatus::Active;
        if (data->strartDownloadTime.isEmpty()) {
            data->strartDownloadTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
        }
        QDateTime createTime = QDateTime::fromString(data->strartDownloadTime, "yyyy-MM-dd hh:mm:ss");
        createTime = createTime.addSecs(15);
        QDateTime currentTime = QDateTime::currentDateTime();
        if ((!QFileInfo::exists(data->savePath)) && (createTime < currentTime)
            && (!fileName.contains("[METADATA]")) && (!data->strartDownloadTime.isEmpty())) {
            return excuteFileNotExist(data, fileName, taskId);
        }
    } else if (statusStr == "waiting") {
        status = Global::DownloadTaskStatus::Waiting;
        downloadSpeed = -1;
    } else if (statusStr == "paused") {
        status = Global::DownloadTaskStatus::Paused;
        downloadSpeed = -2;
    } else if (statusStr == "error") {
        if(errorCode == "12") {
            status = Global::DownloadTaskStatus::Active;
            downloadSpeed = 0;
            QTimer::singleShot(1000, this, [=]() {
                DownloadDataItem *dataItem = m_DownloadTableView->getTableModel()->find(taskId);
                if(dataItem != nullptr && dataItem->taskId != nullptr && dataItem->fileName != nullptr && dataItem->savePath != nullptr)
                    emit unPauseTask(dataItem);
            });
        } else {
            status = Global::DownloadTaskStatus::Error;
            downloadSpeed = -3;
            dealNotificaitonSettings(statusStr, fileName, errorCode);
        }
    } else if (statusStr == "complete") {
        data->status = Global::DownloadTaskStatus::Complete;
        status = Global::DownloadTaskStatus::Complete;
        if (fileName.endsWith(".torrent")) { //自动下载种子文件
            data->status = Global::DownloadTaskStatus::Complete;
            if (Settings::getInstance()->getAutoOpenBtTaskState()) {
                QTimer::singleShot(100, this, [=]() {
                    emit AutoDownloadBt(filePath);
                });
                clearShardMemary();
            }
        } else if (fileName.endsWith(".metalink")) { //自动下载metalink文件
            data->status = Global::DownloadTaskStatus::Complete;
            if (Settings::getInstance()->getAutoOpenMetalinkTaskState()) {
                QTimer::singleShot(100, this, [=]() {
                    emit AutoDownloadMetalink(filePath);
                });
                clearShardMemary();
            }
        }
        //下载文件为磁链种子文件
        QString infoHash = result.value("infoHash").toString();
        bool isMetaData = false;
        if (fileName.startsWith("[METADATA]")) {
            isMetaData = true;
            QString dir = result.value("dir").toString();
            data->status = Global::DownloadTaskStatus::Complete;
            fileName = infoHash + ".torrent";
            filePath = dir + "/" + fileName;
            data->savePath = dir + "/" + fileName;
            data->fileName = fileName;
            //if(Settings::getInstance()->getAutoOpennewTaskWidgetState()){
            if (QFile::exists(filePath)) {
                QTimer::singleShot(100, this, [=]() {
                    emit AutoDownloadBt(dir + "/" + infoHash + ".torrent");
                });
            } else {
                QTimer::singleShot(3000, this, [=]() {
                    emit AutoDownloadBt(dir + "/" + infoHash + ".torrent");
                });
            }
            //}
        }
        //
        dealNotificaitonSettings(statusStr, fileName, errorCode);
        if (Settings::getInstance()->getDownloadFinishedOpenState() && (!isMetaData) && (!fileName.endsWith(".torrent"))) {
            QString urlDecode = QUrl::fromPercentEncoding(filePath.toUtf8());
            urlDecode = "file:///" + urlDecode;
            QUrl url = QUrl(urlDecode, QUrl::TolerantMode);
            QDesktopServices::openUrl(url);
        }
        if (!checkTaskStatus()) {
            QTimer::singleShot(100, [=]() {
                emit whenDownloadFinish();
            });
        }

        if (data->url.isEmpty()) { //bt下载完，有时候会在创建aria2文件
            Aria2RPCInterface::instance()->forceRemove(data->gid, data->taskId);
            QTimer::singleShot(3000, [=]() {
                QFile::remove(filePath + ".aria2");
            });
        }

    } else if (statusStr == "removed") {
        status = Global::DownloadTaskStatus::Removed;
    }
    if (nullptr == m_DownloadTableView->getTableModel()->find(taskId)) {
        return false;
    }
    data->gid = gId;
    if (totalLength > 0) {
        data->totalLength = formatFileSize(totalLength);
    }
    if (completedLength > 0) {
        data->completedLength = formatFileSize(completedLength);
    }
    data->speed = (downloadSpeed != 0) ? formatDownloadSpeed(downloadSpeed) : "0KB/s";
    if (bittorrent.isEmpty()) {
        if (!fileName.isEmpty() && (data->fileName != fileName)) {
            data->fileName = fileName;
        }
        //                if(data->fileName==QObject::tr("Unknown"))
        //                {
        //                    data->fileName = (fileName.isEmpty()) ?
        // Global::UNKNOWN : fileName;
        //                }
        data->status = status;
    } else {
        if ((totalLength != 0) && (totalLength == completedLength)) {
            data->status = DownloadTaskStatus::Complete;
            dealNotificaitonSettings("complete", filePath, errorCode);
        } else {
            data->status = status;
        }
        fileUri = "";
    }
    data->percent = percent;
    data->total = static_cast<int>(totalLength);
    if (!filePath.isEmpty()) {
        data->savePath = filePath;
    } else {
        data->savePath = getDownloadSavepathFromConfig() + data->fileName;
    }
    data->url = fileUri;
    data->time = "";
    if ((totalLength != completedLength) && (totalLength != 0)
        && (data->status == Global::DownloadTaskStatus::Active)) {
        QTime t(0, 0, 0);
        double d = 0;
        if (downloadSpeed > 0) {
            d = (totalLength - completedLength * 1.0) / downloadSpeed;
        }
        if (d > 60 * 60 * 24) {
            data->time = tr(" > 1 day");
        } else {
            t = t.addSecs(static_cast<int>(d));
            data->time = t.toString("hh:mm:ss");
        }
    } else if ((totalLength == 0) && (data->status == Global::DownloadTaskStatus::Active)) {
        data->time = ("--:--");
    } else if (data->time.isEmpty()) {
        data->time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    }
    TaskInfo task;
    TaskInfo getTask;
    DBInstance::getTaskByID(taskId, getTask);
    if ((!getTask.taskId.isEmpty()) && (!getTask.url.isEmpty())) {
        data->url = getTask.url;
    }
    //    m_DownloadTableView->update();
    //    m_DownloadTableView->reset();
    TaskStatus getTaskStatus;
    DBInstance::getTaskStatusById(data->taskId, getTaskStatus);
    QDateTime getTime = QDateTime::currentDateTime();
    TaskStatus saveTaskStatus(data->taskId,
                              data->status,
                              getTime,
                              data->completedLength,
                              data->speed,
                              data->totalLength,
                              data->percent,
                              data->total,
                              getTime);
    if (getTaskStatus.taskId.isEmpty()) {
        DBInstance::addTaskStatus(saveTaskStatus);
    } /* else if ((getTaskStatus.downloadStatus != data->status) || (getTaskStatus.percent != data->speed)) {
            DBInstance::updateTaskStatusById(saveTaskStatus);
        }*/
    m_DownloadTableView->refreshTableView(iCurrentRow);
    return true;
}

bool TableDataControl::aria2MethodShutdown(QJsonObject &json)
{
    QString result = json.value("result").toString();
    if (result == "OK") {
        // m_bShutdownOk = true;
        qDebug() << "close downloadmanager";
        //m_pTableView->close();
        DApplication::exit();
    }
    return true;
}

bool TableDataControl::aria2MethodGetFiles(QJsonObject &json, int iCurrentRow)
{
    Q_UNUSED(json);
    m_DownloadTableView->reset();
    m_DownloadTableView->refreshTableView(iCurrentRow);
    return true;
}

bool TableDataControl::aria2MethodUnpause(QJsonObject &json, int iCurrentRow)
{
    QString gId = json.value("result").toString();
    QString taskId = json.value("id").toString();
    DownloadDataItem *data = m_DownloadTableView->getTableModel()->find(taskId);
    if (data != nullptr) {
        data->status = Global::DownloadTaskStatus::Active;
        m_DownloadTableView->refreshTableView(iCurrentRow);
    } else {
        return false;
    }
    return true;
}

bool TableDataControl::aria2MethodUnpauseAll(QJsonObject &json, int iCurrentRow)
{
    Q_UNUSED(json);
    const QList<Global::DownloadDataItem *> &pItemList = m_DownloadTableView->getTableModel()->dataList();
    for (DownloadDataItem *pItem : pItemList) {
        if (pItem->status != Global::DownloadTaskStatus::Complete) {
            pItem->status = Global::DownloadTaskStatus::Active;
        }
        m_DownloadTableView->refreshTableView(iCurrentRow);
    }
    return true;
}

bool TableDataControl::aria2GetGlobalStatus(QJsonObject &json)
{
    static QList<long long> speedList;
    QJsonObject ja = json.value("result").toObject();
    long long speed = ja.value("downloadSpeed").toString().toLong();
    speedList.append(speed);
    if (speedList.count() >= 5) {
        long long aveSpeed = speedList.at(0) + speedList.at(1) + speedList.at(2) + speedList.at(3) + speedList.at(4);
        aveSpeed /= 5;
        QString speedStr;
        if (Settings::getInstance()->getAutoDownloadBySpeed(speedStr)) {
            if ((aveSpeed / 1024) < speedStr.toInt()) {
                emit addMaxDownloadTask(1);
            }
        }
        speedList.clear();
    }
    return true;
}

bool TableDataControl::aria2MethodForceRemove(QJsonObject &json)
{
    QString id = json.value("id").toString();
    qDebug() << "aria2MethodForceRemove: " << id;
    if (id.startsWith("REDOWNLOAD_")) { // 重新下载前的移除完成后
        QStringList sp = id.split("_");
        if (sp.size() >= 3) {
            QString taskId = sp.at(2);
            int rd = sp.at(1).toInt();
            QThread::msleep(100);
            emit RedownloadJob(taskId, rd);
        }
    }
    return true;
}

bool TableDataControl::saveDataBeforeClose()
{
    const QList<DownloadDataItem *> &dataList = m_DownloadTableView->getTableModel()->dataList();
    const QList<DeleteDataItem *> &recyclelist = m_DownloadTableView->getTableModel()->recyleList();
    for (int j = 0; j < recyclelist.size(); j++) {
        DeleteDataItem *pDelData = recyclelist.at(j);
        QDateTime deltime = QDateTime::fromString(pDelData->deleteTime, "yyyy-MM-dd hh:mm:ss");
        TaskInfo task(pDelData->taskId, pDelData->gid, 0, pDelData->url, pDelData->savePath,
                      pDelData->fileName, deltime);
        DBInstance::updateTaskInfoByID(task);
    }
    for (int i = 0; i < dataList.size(); i++) {
        DownloadDataItem *data = dataList.at(i);
        QDateTime time = QDateTime::fromString(data->createTime, "yyyy-MM-dd hh:mm:ss");
        TaskInfo task(data->taskId, data->gid, 0, data->url, data->savePath,
                      data->fileName, time);
        DBInstance::updateTaskInfoByID(task);
        QDateTime finishTime;
        if (data->status == Global::DownloadTaskStatus::Complete) {
            finishTime = QDateTime::fromString(data->time, "yyyy-MM-dd hh:mm:ss");
        } else {
            finishTime = QDateTime::currentDateTime();
        }
        TaskStatus getStatus;
        int status;
        if ((data->status == Global::DownloadTaskStatus::Complete) || (data->status == Global::DownloadTaskStatus::Removed)) {
            status = data->status;
        } else {
            status = Global::DownloadTaskStatus::Lastincomplete;
        }
        TaskStatus downloadStatus(data->taskId, status, finishTime, data->completedLength, data->speed,
                                  data->totalLength, data->percent, data->total, finishTime);
        if (DBInstance::getTaskStatusById(data->taskId, getStatus)) {
            DBInstance::updateTaskStatusById(downloadStatus);
        } else {
            DBInstance::addTaskStatus(downloadStatus);
        }
    }
    return true;
}

bool TableDataControl::updateDb()
{
    const QList<DownloadDataItem *> &dataList = m_DownloadTableView->getTableModel()->dataList();
    QList<TaskInfo> infoList;
    QList<TaskStatus> statusList;
    for (int i = 0; i < dataList.size(); i++) {
        DownloadDataItem *data = dataList.at(i);
        QDateTime time = QDateTime::fromString(data->createTime, "yyyy-MM-dd hh:mm:ss");
        TaskInfo task(data->taskId, data->gid, 0, data->url, data->savePath,
                      data->fileName, time);
        //DBInstance::updateTaskInfoByID(task);
        infoList.append(task);
        QDateTime finishTime;
        if (data->status == Global::DownloadTaskStatus::Complete) {
            finishTime = QDateTime::fromString(data->time, "yyyy-MM-dd hh:mm:ss");
        } else {
            finishTime = QDateTime::currentDateTime();
        }
        TaskStatus getStatus;
        TaskStatus downloadStatus(data->taskId, data->status, finishTime, data->completedLength, data->speed,
                                  data->totalLength, data->percent, data->total, finishTime);
        if (DBInstance::getTaskStatusById(data->taskId, getStatus)) {
            //DBInstance::updateTaskStatusById(downloadStatus);
            statusList.append(downloadStatus);
        } else {
            DBInstance::addTaskStatus(downloadStatus);
        }
    }
    DBInstance::updateAllTaskInfo(infoList);
    DBInstance::updateAllTaskStatus(statusList);
    return true;
}

QString TableDataControl::getFileName(const QString &url)
{
    return QString(url).right(url.length() - url.lastIndexOf('/') - 1);
}

void TableDataControl::dealNotificaitonSettings(QString statusStr, QString fileName, QString errorCode)
{
    bool downloadInfoNotify = Settings::getInstance()->getDownloadInfoSystemNotifyState();
    if (!downloadInfoNotify) {
        return;
    }
    QDBusInterface tInterNotify("com.deepin.dde.Notification",
                                "/com/deepin/dde/Notification",
                                "com.deepin.dde.Notification",
                                QDBusConnection::sessionBus());
    QList<QVariant> arg;
    QString in0("downloader"); //下载器
    uint in1 = 101;
    QString in2 = "downloader";
    QString in3;
    QString in4;
    QStringList in5;
    QVariantMap in6;
    if (statusStr == "error") {
        in3 = tr("Download failed");
        in4 = QString(tr("%1 download failed. Network error.")).arg(fileName);
        in5 << "_cancel" << tr("Cancel") << "_view" << tr("View");
        in6["x-deepin-action-_view"] = "downloader";
        qDebug() << in4 + ("    errorCode: ") + errorCode;
    } else {
        in3 = tr("Download completed");
        in4 = QString(tr("%1 download finished")).arg(fileName);
    }
    int in7 = 5000;
    arg << in0 << in1 << in2 << in3 << in4 << in5 << in6 << in7;
    tInterNotify.callWithArgumentList(QDBus::AutoDetect, "Notify", arg);
}

QString TableDataControl::formatFileSize(long size)
{
    QString result;
    if (size < 1024) {
        result = QString::number(size) + "B";
    } else if (size / 1024 < 1024) {
        result = QString::number(size * 1.0 / 1024, 'r', 1) + "KB";
    } else if (size / 1024 / 1024 < 1024) {
        result = QString::number(size * 1.0 / 1024 / 1024, 'r', 1) + "MB";
    } else if (size / 1024 / 1024 / 1024 < 1024) {
        result = QString::number(size * 1.0 / 1024 / 1024 / 1024, 'r', 1) + "GB";
    }
    return result;
}

QString TableDataControl::getDownloadSavepathFromConfig()
{
    return Settings::getInstance()->getDownloadSavePath();
}

QString TableDataControl::formatDownloadSpeed(long size)
{
    QString result;
    if (size < 0) {
        result = QString::number(size) + " KB/s";
    } else if (size < 1024) {
        result = QString::number(size) + " B/s";
    } else if (size / 1024 < 1024) {
        result = QString::number(size * 1.0 / 1024, 'r', 1) + " KB/s";
    } else if (size / 1024 / 1024 < 1024) {
        result = QString::number(size * 1.0 / 1024 / 1024, 'r', 1) + " MB/s";
    } else if (size / 1024 / 1024 / 1024 < 1024) {
        result = QString::number(size * 1.0 / 1024 / 1024 / 1024, 'r', 1) + " GB/s";
    }
    return result;
}

void TableDataControl::onUnusualConfirm(int index, const QString &taskIds)
{
    for (QString taskId : taskIds.split("\n")) {
        DownloadDataItem *pItem = m_DownloadTableView->getTableModel()->find(taskId);
        if (nullptr == pItem) {
            continue;
        }
        TaskInfoHash info;
        bool isBtTask = false;
        if (pItem->url.isEmpty()) {
            DBInstance::getBtTaskById(taskId, info);
            isBtTask = !info.taskId.isEmpty();
        }
        if (0 == index) {
            if (isBtTask) {
                QMap<QString, QVariant> opt;
                QString path = pItem->savePath.left(pItem->savePath.lastIndexOf("/"));
                opt.insert("dir", path);
                opt.insert("select-file", info.selectedNum);
                QString fileName = pItem->fileName;
                removeDownloadListJob(pItem, false, false);
                emit DownloadUnusuaBtJob(info.filePath, opt, fileName, info.infoHash);
            } else {
                QString url = pItem->url;
                QString savepath = pItem->savePath.left(pItem->savePath.lastIndexOf("/"));
                QMimeDatabase db;
                QString mime = db.suffixForFileName(pItem->fileName);
                QString fileName = pItem->fileName.mid(0, pItem->fileName.lastIndexOf(mime) - 1);
                QString totalLength = pItem->totalLength;
                removeDownloadListJob(pItem, false, false);
                emit DownloadUnusuaHttpJob(url, savepath, fileName, mime, totalLength);
            }
        } else {
            removeDownloadListJob(pItem);
        }
    }
}

bool TableDataControl::searchEditTextChanged(QString text, QList<QString> &taskIDList,
                                             QList<int> &taskStatusList, QList<QString> &tasknameList)
{
    TableModel *pModel = m_DownloadTableView->getTableModel();
    for (const DownloadDataItem *pItem : pModel->dataList()) {
        QString fileName = pItem->fileName;
        QString namePinyin = Func::chineseToPinyin(fileName);
        if ((fileName.contains(text, Qt::CaseInsensitive)) || namePinyin.contains(text, Qt::CaseInsensitive)) {
            taskIDList.append(pItem->taskId);
            taskStatusList.append(pItem->status);
            tasknameList.append(pItem->fileName);
        }
    }
    for (const DeleteDataItem *pItem : pModel->recyleList()) {
        QString fileName = pItem->fileName;
        QString namePinyin = Func::chineseToPinyin(fileName);
        if ((fileName.contains(text, Qt::CaseInsensitive)) || namePinyin.contains(text, Qt::CaseInsensitive)) {
            taskIDList.append(pItem->taskId);
            taskStatusList.append(pItem->status);
            tasknameList.append(pItem->fileName);
        }
    }
    return true;
}

bool TableDataControl::onDelAction(int currentTab)
{
    if (currentTab == Global::recycleTab) {
        m_RecycleDeleteList.clear();
        const QList<DeleteDataItem *> &pList = m_DownloadTableView->getTableModel()->recyleList();
        for (int i = 0; i < pList.size(); ++i) {
            if (pList.at(i)->isChecked) {
                m_RecycleDeleteList.append(pList.at(i));
            }
        }
    } else {
        m_DeleteList.clear();
        const QList<DownloadDataItem *> &pSelectList = m_DownloadTableView->getTableModel()->renderList();
        for (int i = 0; i < pSelectList.size(); ++i) {
            if ((currentTab == Global::finishTab && pSelectList.at(i)->status == Complete) || (currentTab == Global::downloadingTab && pSelectList.at(i)->status != Complete)) {
                if (pSelectList.at(i)->isChecked) {
                    m_DeleteList.append(pSelectList.at(i));
                }
            }
        }
    }
    return true;
}

bool TableDataControl::onDeleteDownloadListConfirm(bool ischecked, bool permanent, TableView *pRecycleTableView)
{
    bool ifDeleteLocal = permanent || ischecked;
    DeleteItemThread *pDeleteItemThread = new DeleteItemThread(m_DeleteList,
                                                               m_DownloadTableView,
                                                               ifDeleteLocal,
                                                               "download_delete");
    pDeleteItemThread->setParent(this);
    connect(pDeleteItemThread, &DeleteItemThread::Aria2Remove, this, [](QString gId, QString id) {
        Aria2RPCInterface::instance()->forceRemove(gId, id);
    });
    connect(pDeleteItemThread, &DeleteItemThread::removeFinished, this, [=]() {
        deleteTask(permanent, pRecycleTableView);
        emit removeFinished();
        //delete pDeleteItemThread;
    });

    pDeleteItemThread->start();
    return true;
}

bool TableDataControl::onDeleteRecycleListConfirm(bool ischecked, bool permanent)
{
    bool ifDeleteLocal = permanent || ischecked;

    DeleteItemThread *pDeleteItemThread = new DeleteItemThread(m_RecycleDeleteList,
                                                               m_DownloadTableView,
                                                               ifDeleteLocal,
                                                               "recycle_delete");
    pDeleteItemThread->setParent(this);
    connect(pDeleteItemThread, &DeleteItemThread::Aria2Remove, [=](QString gId, QString id) {
        Aria2RPCInterface::instance()->forceRemove(gId, id);
    });
    connect(pDeleteItemThread, &DeleteItemThread::removeFinished, this, [&]() {
        for (int i = 0; i < m_RecycleDeleteList.size(); i++) {
            DeleteDataItem *data = m_RecycleDeleteList.at(i);
            DBInstance::delTask(data->taskId);
            m_DownloadTableView->getTableModel()->removeItem(data);
        }
        if (m_DownloadTableView->getTableModel()->recyleList().isEmpty()) {
            m_DownloadTableView->getTableHeader()->onHeaderChecked(false);
        }
        emit removeFinished();
        //delete pDeleteItemThread;
    });
    pDeleteItemThread->start();
    return true;
}

bool TableDataControl::downloadListRedownload(QString id)
{
    DownloadDataItem *data = m_DownloadTableView->getTableModel()->find(id);
    if (data == nullptr) {
        return false;
    }
    reDownloadTask(data->taskId, data->savePath, data->fileName, data->url);
    return true;
}

bool TableDataControl::recycleListRedownload(QString id)
{
    DeleteDataItem *data = m_DownloadTableView->getTableModel()->find(id, 2);
    if (data == nullptr) {
        return false;
    }
    reDownloadTask(data->taskId, data->savePath, data->fileName, data->url);
    return true;
}

void TableDataControl::clearShardMemary()
{
    QSharedMemory sharedMemory;
    sharedMemory.setKey("downloader");
    if (sharedMemory.attach()) //设置成单例程序
    {
        sharedMemory.lock();
        char *to = static_cast<char *>(sharedMemory.data());
        int num = sharedMemory.size();
        memset(to, 0, num);
        sharedMemory.unlock();
    }
}

bool TableDataControl::checkTaskStatus()
{
    const QList<DownloadDataItem *> &dataList = m_DownloadTableView->getTableModel()->dataList();
    for (const auto *item : dataList) {
        if ((item->status == Global::DownloadTaskStatus::Active) || (item->status == Global::DownloadTaskStatus::Waiting)) {
            return true;
        }
    }
    return false;
}

bool TableDataControl::deleteTask(bool ifDeleteLocal, TableView *pRecycleTableView)
{
    QString gid;
    QString ariaTempFile;
    QString savePath;
    QString taskId;

    for (int i = 0; i < m_DeleteList.size(); i++) {
        DownloadDataItem *data = m_DeleteList.at(i);
        savePath = data->savePath;
        gid = data->gid;
        taskId = data->taskId;
        QDateTime finishTime;
        if (data->status == Complete) {
            finishTime = QDateTime::fromString(data->time, "yyyy-MM-dd hh:mm:ss");
        } else {
            finishTime = QDateTime::fromString("", "yyyy-MM-dd hh:mm:ss");
        }
        TaskStatus getStatus;
        TaskStatus downloadStatus(data->taskId,
                                  Global::DownloadTaskStatus::Removed,
                                  QDateTime::currentDateTime(),
                                  data->completedLength,
                                  data->speed,
                                  data->totalLength,
                                  data->percent,
                                  data->total,
                                  finishTime);
        if (ifDeleteLocal) {
            TaskInfoHash info;
            DBInstance::getBtTaskById(taskId, info);
            if (info.downloadType == "torrent") {
                clearShardMemary();
            }
            DBInstance::delTask(taskId);
            //Aria2RPCInterface::instance()->purgeDownloadResult(data->gid);
        }
        if (!ifDeleteLocal) {
            DeleteDataItem *delData = new DeleteDataItem;
            delData->taskId = data->taskId;
            delData->gid = data->gid;
            delData->url = data->url;
            delData->fileName = data->fileName;
            delData->savePath = data->savePath;
            delData->status = DownloadTaskStatus::Removed;
            delData->isChecked = false;
            delData->totalLength = data->totalLength;
            delData->completedLength = data->completedLength;
            delData->deleteTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
            delData->finishTime = data->time;
            pRecycleTableView->getTableModel()->append(delData);
            m_DownloadTableView->update();
            if (DBInstance::getTaskStatusById(delData->taskId, getStatus)) {
                DBInstance::updateTaskStatusById(downloadStatus);
            } else {
                DBInstance::addTaskStatus(downloadStatus);
            }
        }
        m_DownloadTableView->getTableModel()->removeItem(data);
        QThread::usleep(10);
    }
    if (m_DownloadTableView->getTableModel()->recyleList().isEmpty()) {
        m_DownloadTableView->getTableHeader()->onHeaderChecked(false);
    }
    pRecycleTableView->update();
    return true;
}

bool TableDataControl::reDownloadTask(QString taskId, QString filePath, QString fileName, QString url)
{
    QString savePath = getDownloadSavepathFromConfig();
    if (getDownloadSavepathFromConfig() != filePath) {
        int folderPathLength = filePath.size() - fileName.size() - 1;
        savePath = filePath.left(folderPathLength);
    }
    QUuid uuid = QUuid::createUuid();
    QString strId = uuid.toString();
    TaskInfoHash taskInfo;
    DBInstance::getBtTaskById(taskId, taskInfo);
    if (!taskInfo.taskId.isEmpty()) {
        if (taskInfo.downloadType == "torrent") {
            if (!taskInfo.infoHash.isEmpty()) {
                QFile::remove(taskInfo.infoHash + ".torrent");
            }
            QMap<QString, QVariant> opt;
            opt.insert("dir", savePath);
            opt.insert("select-file", taskInfo.selectedNum);
            // aria2c->addTorrent(get_UrlInfo->seedFile,opt,get_UrlInfo->task_id);
            TaskInfo addTask(taskInfo.taskId,
                             "",
                             0,
                             "",
                             "",
                             fileName,
                             QDateTime::currentDateTime());
            DBInstance::addTask(addTask);
            Aria2RPCInterface::instance()->addTorrent(taskInfo.filePath, opt, taskInfo.taskId);
        }
    } else {
        QMap<QString, QVariant> opt;
        opt.insert("dir", savePath);
        opt.insert("out", fileName);
        Aria2RPCInterface::instance()->addUri(url, opt, strId);
        QString filename = QString(url).right(url.length() - url.lastIndexOf('/') - 1);
        if (!filename.contains(QRegExp("[\\x4e00-\\x9fa5]+"))) {
            const QByteArray filenameByte = filename.toLatin1();
            QString filenameDecode = QUrl::fromPercentEncoding(filenameByte);
            filename = filenameDecode;
        }
        TaskInfo addTask(strId, "", 0, url, filePath, filename, QDateTime::currentDateTime());
        DBInstance::addTask(addTask);
    }
    return true;
}

bool TableDataControl::excuteFileNotExist(DownloadDataItem *data, QString filename, QString taskId)
{
    Aria2RPCInterface::instance()->remove(data->gid);
    if (Settings::getInstance()->getAutoDeleteFileNoExistentTaskState()) { // 删除文件不存在的任务
        removeDownloadListJob(data);
        return true;
    }
    data->status = Global::DownloadTaskStatus::Error;
    static QString taskLsit;
    static QString unusualId;

    if (taskLsit.isEmpty()) {
        QTimer::singleShot(500, this, [&]() {
            MessageBox msg(m_DownloadTableView);
            msg.setAccessibleName("unusualMessageBox");
            msg.setUnusual(unusualId, taskLsit);
            connect(&msg, &MessageBox::unusualConfirm, this, &TableDataControl::onUnusualConfirm);
            msg.exec();
            taskLsit.clear();
            unusualId.clear();
        });
    }
    taskLsit.append(filename + "\n");
    unusualId.append(taskId + "\n");
    return true;
}
