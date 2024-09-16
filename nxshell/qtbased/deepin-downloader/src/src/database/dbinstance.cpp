// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dbinstance.h"

bool DBInstance::addTask(TaskInfo &task)
{
    QSqlDatabase q = DataBase::Instance().getDB();
    if (!q.isOpen()) {
        qDebug() << q.lastError();
        return false;
    }
    QSqlQuery sql;
    sql.prepare("insert into download_task values (?,?,?,?,?,?,?,?);");
    sql.addBindValue(task.taskId);
    sql.addBindValue(task.gid);
    sql.addBindValue(task.gidIndex);
    sql.addBindValue(task.url);
    sql.addBindValue(task.downloadPath);
    sql.addBindValue(task.downloadFilename);
    sql.addBindValue(task.createTime);
    sql.addBindValue(task.fileLength);
    if (!sql.exec()) {
      //  qWarning() << "Insert download_task table failed : " << sql.lastError();
        return false;
    }
    return true;
}

bool DBInstance::delTask(QString taskId)
{
    QSqlDatabase q = DataBase::Instance().getDB();
    if (!q.isOpen()) {
        qDebug() << q.lastError();
        return false;
    }
    QSqlQuery sql;
    QString str = QString("delete from  download_task where task_id='%1';").arg(taskId);
    sql.prepare(str);
    if (!sql.exec()) {
//        QSqlError error = sql.lastError();
//        qWarning() << "Delete download_task failed : " << error;
        return false;
    }
    sql.clear();
    sql.prepare("delete from  download_task_status where task_id=?;");
    sql.addBindValue(taskId);
    if (!sql.exec()) {
//        QSqlError error = sql.lastError();
//        qWarning() << "Delete download_task_status failed : " << error;
        return false;
    }
    sql.clear();
    sql.prepare("delete from  url_info where task_id=?;");
    sql.addBindValue(taskId);
    if (!sql.exec()) {
//        QSqlError error = sql.lastError();
//        qWarning() << "Delete url_info failed : " << error;
        return false;
    }
    return true;
}

bool DBInstance::delAllTask()
{
    QSqlDatabase q = DataBase::Instance().getDB();
    if (!q.isOpen()) {
        qDebug() << q.lastError();
        return false;
    }
    QSqlQuery sql;
    sql.prepare("delete from  download_task;");
    if (!sql.exec()) {
//        qWarning() << "Delete download_task failed : " << sql.lastError();
        return false;
    }
    sql.clear();
    sql.prepare("delete from download_task_status;");
    if (!sql.exec()) {
//        qWarning() << "Delete download_task failed : " << sql.lastError();
        return false;
    }
    sql.clear();
    sql.prepare("delete from url_info;");
    if (!sql.exec()) {
//        qWarning() << "Delete url_info failed : " << sql.lastError();
        return false;
    }
    return true;
}

bool DBInstance::updateTaskInfoByID(TaskInfo &task)
{
    QSqlDatabase q = DataBase::Instance().getDB();
    if (!q.isOpen()) {
        qDebug() << q.lastError();
        return false;
    }
    QSqlQuery sql;
    sql.prepare("update  download_task set  gid=? , gid_index=? , url=? ,download_path=? ,"
                " download_filename=? ,create_time=? ,file_length=? where task_id= ?");
    sql.addBindValue(task.gid);
    sql.addBindValue(task.gidIndex);
    sql.addBindValue(task.url);
    sql.addBindValue(task.downloadPath);
    sql.addBindValue(task.downloadFilename);
    sql.addBindValue(task.createTime);
    sql.addBindValue(task.fileLength);
    sql.addBindValue(task.taskId);

    if (!sql.exec()) {
        qWarning() << "Update download_task table failed : " << sql.lastError();
        return false;
    }
    return true;
}

bool DBInstance::updateAllTaskInfo(QList<TaskInfo> &taskList)
{
    QSqlDatabase q = DataBase::Instance().getDB();
    if (!q.isOpen()) {
        qDebug() << q.lastError();
        return false;
    }
    for (TaskInfo task : taskList) {
        QSqlQuery sql;
        sql.prepare("update  download_task set  gid=? , gid_index=? , url=? ,download_path=? , download_filename=? ,create_time=? ,file_length=? where task_id= ?");
        sql.addBindValue(task.gid);
        sql.addBindValue(task.gidIndex);
        sql.addBindValue(task.url);
        sql.addBindValue(task.downloadPath);
        sql.addBindValue(task.downloadFilename);
        sql.addBindValue(task.createTime);
        sql.addBindValue(task.taskId);
        sql.addBindValue(task.fileLength);

        if (!sql.exec()) {
//            qWarning() << "Update download_task table failed : " << sql.lastError();
            return false;
        }
    }
    return true;
}

bool DBInstance::getTaskByID(QString taskId, TaskInfo &task)
{
    QSqlDatabase q = DataBase::Instance().getDB();
    if (!q.isOpen()) {
        qDebug() << q.lastError();
        return false;
    }
    QSqlQuery sql;
    sql.prepare("select * from download_task where task_id=:index;");
    sql.addBindValue(taskId);
    if (!sql.exec()) {
//        qDebug() << "Select download_task table failed : " << sql.lastError();
        return false;
    }
    while (sql.next()) {
        task.taskId = sql.value(0).toString();
        task.gid = sql.value(1).toString(); //下载gid
        task.gidIndex = sql.value(2).toInt(); //位置index
        task.url = sql.value(3).toString(); //下载url地址
        task.downloadPath = sql.value(4).toString(); //下载全路径包括文件名
        task.downloadFilename = sql.value(5).toString(); //下载文件名
        task.createTime = sql.value(6).toDateTime(); //任务创建时间
        task.fileLength = sql.value(7).toString(); //任务大小
    }
    return true;
}

bool DBInstance::getAllTask(QList<TaskInfo> &taskList)
{
    QSqlDatabase q = DataBase::Instance().getDB();
    if (!q.isOpen()) {
        qDebug() << q.lastError();
        return false;
    }
    QSqlQuery sql;
    sql.prepare("select * from download_task;");
    if (!sql.exec()) {
//        qDebug() << "getAllTask download_task table failed : " << sql.lastError();

        return false;
    }
    TaskInfo task;
    while (sql.next()) {
        task.taskId = sql.value(0).toString();

        task.gid = sql.value(1).toString(); //下载gid
        task.gidIndex = sql.value(2).toInt(); //位置index
        task.url = sql.value(3).toString(); //下载url地址
        task.downloadPath = sql.value(4).toString(); //下载全路径包括文件名
        task.downloadFilename = sql.value(5).toString(); //下载文件名
        task.createTime = sql.value(6).toDateTime(); //任务创建时间
        task.fileLength = sql.value(7).toString(); //任务大小
        taskList.push_back(task);
    }
    return true;
}

bool DBInstance::isExistUrl(QString url, bool &ret)
{
    ret = false;
    QSqlDatabase q = DataBase::Instance().getDB();
    if (!q.isOpen()) {
        qDebug() << q.lastError();
        return false;
    }
    QSqlQuery sql;
    QString selectAllSql = "select count(*)  from download_task where download_task.url like'" + url + "%' ;";
    sql.prepare(selectAllSql);
    if (!sql.exec()) {
        return false;
    }
    while (sql.next()) {
        if (sql.value(0).toInt() >= 1) {
            ret = true;
        }
    }
    url = url.remove("magnet:?xt=urn:btih:");
    selectAllSql = "select count(*)  from url_info where url_info.infoHash like'%" + url + "%' ;";
    sql.prepare(selectAllSql);
    if (!sql.exec()) {
        return false;
    }
    while (sql.next()) {
        if (sql.value(0).toInt() >= 1) {
            ret = true;
        }
    }
    return true;
}

QString DBInstance::getTaskIdByMagnet(QString url)
{
    QSqlDatabase q = DataBase::Instance().getDB();
    if (!q.isOpen()) {
        qDebug() << q.lastError();
        return "";
    }
    QSqlQuery sql;
    url = url.remove("magnet:?xt=urn:btih:");
    QString selectAllSql = "select task_id  from url_info where url_info.infoHash like'%" + url + "%' ;";
    sql.prepare(selectAllSql);
    if (!sql.exec()) {
        return "";
    }
    while (sql.next()) {
        return sql.value(0).toString();
    }
    return "";
}

bool DBInstance::addTaskStatus(TaskStatus &task)
{
    QSqlDatabase q = DataBase::Instance().getDB();
    if (!q.isOpen()) {
        qDebug() << q.lastError();
        return false;
    }
    QSqlQuery sql;
    sql.prepare("insert into download_task_status values (?,?,?,?,?,?,?,?,?);");
    sql.addBindValue(task.taskId);
    sql.addBindValue(task.downloadStatus);
    sql.addBindValue(task.modifyTime);
    sql.addBindValue(task.compeletedLength);
    sql.addBindValue(task.downloadSpeed);
    sql.addBindValue(task.totalLength);
    sql.addBindValue(task.percent);
    sql.addBindValue(task.totalFromSource);
    sql.addBindValue(task.finishTime);
    if (!sql.exec()) {
//        QSqlError error = sql.lastError();
//        qWarning() << "insert download_task_status failed : " << sql.lastError();
        return false;
    }
    return true;
}

bool DBInstance::updateTaskStatusById(TaskStatus &task)
{
    QSqlDatabase q = DataBase::Instance().getDB();
    if (!q.isOpen()) {
        qDebug() << q.lastError();
        return false;
    }
    QSqlQuery sql;
    sql.prepare("update  download_task_status set  download_status=? , modify_time=? ,compeletedLength=? , download_speed=? , totalLength=? ,percent=? , totalFromSource=? ,finish_time=? where task_id= ?");
    sql.addBindValue(task.downloadStatus);
    sql.addBindValue(task.modifyTime);
    sql.addBindValue(task.compeletedLength);
    sql.addBindValue(task.downloadSpeed);
    sql.addBindValue(task.totalLength);
    sql.addBindValue(task.percent);
    sql.addBindValue(task.totalFromSource);
    sql.addBindValue(task.finishTime);
    sql.addBindValue(task.taskId);
    if (!sql.exec()) {
//        qWarning() << "update download_task_status failed : " << sql.lastError();
        return false;
    }
    return true;
}

bool DBInstance::updateAllTaskStatus(QList<TaskStatus> &taskList)
{
    QSqlDatabase q = DataBase::Instance().getDB();
    if (!q.isOpen()) {
        qDebug() << q.lastError();
        return false;
    }
    for (TaskStatus task : taskList) {
        QSqlQuery sql;
        sql.prepare("update  download_task_status set  download_status=? , modify_time=? ,compeletedLength=? , download_speed=? , totalLength=? ,percent=? , totalFromSource=? ,finish_time=? where task_id= ?");
        sql.addBindValue(task.downloadStatus);
        sql.addBindValue(task.modifyTime);
        sql.addBindValue(task.compeletedLength);
        sql.addBindValue(task.downloadSpeed);
        sql.addBindValue(task.totalLength);
        sql.addBindValue(task.percent);
        sql.addBindValue(task.totalFromSource);
        sql.addBindValue(task.finishTime);
        sql.addBindValue(task.taskId);
        if (!sql.exec()) {
//            qWarning() << "update download_task_status failed : " << sql.lastError();
            return false;
        }
    }
    return true;
}

bool DBInstance::getTaskStatusById(QString taskId, TaskStatus &task)
{
    QSqlDatabase q = DataBase::Instance().getDB();
    if (!q.isOpen()) {
        qDebug() << q.lastError();
        return false;
    }
    QSqlQuery sql;
    QString selectAllAql = "select * from download_task_status where task_id='" + taskId + "';";
    sql.prepare(selectAllAql);
    if (!sql.exec()) {
//        qWarning() << "update download_task_status failed : " << sql.lastError();
        return false;
    }
    while (sql.next()) {
        task.taskId = sql.value(0).toString();
        task.downloadStatus = sql.value(1).toInt(); //下载状态
        task.modifyTime = sql.value(2).toDateTime();
        task.compeletedLength = sql.value(3).toString();
        task.downloadSpeed = sql.value(4).toString();
        task.totalLength = sql.value(5).toString();
        task.percent = sql.value(6).toInt();
        task.totalFromSource = sql.value(7).toInt();
        task.finishTime = sql.value(8).toDateTime();
    }
    return true;
}

bool DBInstance::getAllTaskStatus(QList<TaskStatus> &taskList)
{
    QSqlDatabase q = DataBase::Instance().getDB();
    if (!q.isOpen()) {
        qDebug() << q.lastError();
        return false;
    }
    QString selectAllAql = "select * from download_task_status;";
    QSqlQuery sql;
    sql.prepare(selectAllAql);
    if (!sql.exec()) {
//        qWarning() << "get all download_task_status failed : " << sql.lastError();
        return false;
    }
    TaskStatus task;
    while (sql.next()) {
        task.taskId = sql.value(0).toString();
        task.downloadStatus = sql.value(1).toInt(); //下载状态
        task.modifyTime = sql.value(2).toDateTime();
        task.compeletedLength = sql.value(3).toString();
        task.downloadSpeed = sql.value(4).toString();
        task.totalLength = sql.value(5).toString();
        task.percent = sql.value(6).toInt();
        task.totalFromSource = sql.value(7).toInt();
        task.finishTime = sql.value(8).toDateTime();
        taskList.push_back(task);
    }
    return true;
}

bool DBInstance::addBtTask(TaskInfoHash &url)
{
    QSqlDatabase q = DataBase::Instance().getDB();
    if (!q.isOpen()) {
        qDebug() << q.lastError();
        return false;
    }
    QSqlQuery sql;
    sql.prepare("insert into url_info values (?,?,?,?,?,?);");
    sql.addBindValue(url.taskId);
    sql.addBindValue(url.url);
    sql.addBindValue(url.downloadType);
    sql.addBindValue(url.filePath);
    sql.addBindValue(url.selectedNum);
    sql.addBindValue(url.infoHash);
    if (!sql.exec()) {
//        qWarning() << "insert url_info failed : " << sql.lastError();
        return false;
    }
    return true;
}

bool DBInstance::updateBtTaskById(TaskInfoHash &url)
{
    QSqlDatabase q = DataBase::Instance().getDB();
    if (!q.isOpen()) {
        qDebug() << q.lastError();
        return false;
    }
    QSqlQuery sql;
    sql.prepare("update  url_info set url=?,download_type=?,seedFile=?,selectedNum=? ,infoHash=? where task_id= ?");
    sql.addBindValue(url.url);
    sql.addBindValue(url.downloadType);
    sql.addBindValue(url.filePath);
    sql.addBindValue(url.selectedNum);
    sql.addBindValue(url.infoHash);
    sql.addBindValue(url.taskId);
    return true;
}

bool DBInstance::getBtTaskById(QString taskId, TaskInfoHash &url)
{
    QSqlDatabase q = DataBase::Instance().getDB();
    if (!q.isOpen()) {
        qDebug() << q.lastError();
        return false;
    }
    QSqlQuery sql;
    QString selectAllAql = "select * from url_info where task_id='" + taskId + "';";
    sql.prepare(selectAllAql);
    if (!sql.exec()) {
//        qWarning() << "select url_info failed : " << sql.lastError();
        return false;
    }
    while (sql.next()) {
        url.taskId = sql.value(0).toString(); //任务id
        url.url = sql.value(1).toString(); // url 下载地址
        url.downloadType = sql.value(2).toString(); //下载类型
        url.filePath = sql.value(3).toString(); //种子文件
        url.selectedNum = sql.value(4).toString(); //选择的种子文件号码
        url.infoHash = sql.value(5).toString(); //种子文件hash值
    }
    return true;
}

bool DBInstance::getAllBtTask(QList<TaskInfoHash> &urlList)
{
    QSqlDatabase q = DataBase::Instance().getDB();
    if (!q.isOpen()) {
        qDebug() << q.lastError();
        return false;
    }
    QSqlQuery sql;
    QString selectAllSql = "select * from url_info;";
    sql.prepare(selectAllSql);
    if (!sql.exec()) {
//        qDebug() << sql.lastError();
        return false;
    }
    TaskInfoHash url;
    while (sql.next()) {
        url.taskId = sql.value(0).toString(); //任务id
        url.url = sql.value(1).toString(); // url 下载地址
        url.downloadType = sql.value(2).toString(); //下载类型
        url.filePath = sql.value(3).toString(); //种子文件
        url.selectedNum = sql.value(4).toString(); //选择的种子文件号码
        url.infoHash = sql.value(5).toString(); //种子文件hash值
        urlList.push_back(url);
    }
    return true;
}

int DBInstance::getSameNameCount(QString filename, QString type)
{
    int count = 0;
    QSqlDatabase q = DataBase::Instance().getDB();
    if (!q.isOpen()) {
        qDebug() << q.lastError();
        return false;
    }

    QSqlQuery sql;
    QString sqlStr = QString("select count(download_filename) from download_task where"
                             " download_filename = '"
                             + filename + "." + type + "';");
    sql.prepare(sqlStr);
    if (!sql.exec()) {
//        qWarning() << "select count(download_filename) failed : " << sql.lastError();
//        qWarning() << sqlStr;
        return 0;
    }
    qWarning() << sqlStr;
    while (sql.next()) {
        count = sql.value(0).toInt();
    }
    if (count == 1) {
        QSqlQuery sql1;
        QString sqlStr1 = QString("select count(download_filename) from download_task where"
                                  " download_filename like '"
                                  + filename + "-%." + type + "';");
        sql1.prepare(sqlStr1);
        if (!sql1.exec()) {
//            qWarning() << "select count(download_filename) failed : " << sql.lastError();
//            qWarning() << sqlStr1;
            return 0;
        }
        while (sql1.next()) {
            count += sql1.value(0).toInt();
        }
    }
    return count;
}

bool DBInstance::isExistBtInHash(QString hash, bool &ret)
{
    ret = false;
    QSqlDatabase q = DataBase::Instance().getDB();
    if (!q.isOpen()) {
        qDebug() << q.lastError();
        return false;
    }
    QSqlQuery sql;
    QString selectAllSql = "select count(*)  from url_info where url_info.infoHash='" + hash + "' ;";
    qDebug() << selectAllSql;
    sql.prepare(selectAllSql);
    if (!sql.exec()) {
//        qWarning() << "select download_task,download_task_status failed :  " << sql.lastError();
        return false;
    }
    while (sql.next()) {
        if (sql.value(0).toInt() >= 1) {
            ret = true;
        }
    }
    return true;
}

bool DBInstance::getTaskForUrl(QString url, TaskInfo &task)
{
    QSqlDatabase q = DataBase::Instance().getDB();
    if (!q.isOpen()) {
        qDebug() << q.lastError();
        return false;
    }
    QSqlQuery sql;
    QString str = QString("select * from download_task where url = '%1';").arg(url);
    sql.prepare(str);
    if (!sql.exec()) {
//        qDebug() << "getAllTask download_task table failed : " << sql.lastError();
        return false;
    }
    while (sql.next()) {
        task.taskId = sql.value(0).toString();
        task.gid = sql.value(1).toString(); //下载gid
        task.gidIndex = sql.value(2).toInt(); //位置index
        task.url = sql.value(3).toString(); //下载url地址
        task.downloadPath = sql.value(4).toString(); //下载全路径包括文件名
        task.downloadFilename = sql.value(5).toString(); //下载文件名
        task.createTime = sql.value(6).toDateTime(); //任务创建时间
        task.fileLength = sql.value(7).toString();
    }
    return true;
}
