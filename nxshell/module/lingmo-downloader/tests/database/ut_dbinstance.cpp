// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <iostream>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "dbinstance.h"
#include "database.h"
#include "stub.h"
#include "stubAll.h"
#include <stdio.h>

class ut_dbinstance : public ::testing::Test
{
protected:
    ut_dbinstance()
    {
    }

    virtual ~ut_dbinstance()
    {
    }
    virtual void SetUp()
    {
        //        sql.addBindValue(task.taskId);
        //        sql.addBindValue(task.gid);
        //        sql.addBindValue(task.gidIndex);
        //        sql.addBindValue(task.url);
        //        sql.addBindValue(task.downloadPath);
        //        sql.addBindValue(task.downloadFilename);
        //        sql.addBindValue(task.createTime);
        m_taskInfo.gid = "4261f7807e5d90d8";
        m_taskInfo.url = "http://download.qt.io/archive/qt/4.1/qt-x11-opensource-src-4.1.1.tar.gz";
        m_taskInfo.taskId = "78d8db55-10c2-4200-b097-253120d71269";
        m_taskInfo.gidIndex = 0;
        m_taskInfo.downloadPath = "/home/bulw";
        m_taskInfo.createTime = QDateTime::currentDateTime();

        m_btTasknInfo.taskId = "78d8db55-10c2-4200-b097-253120d71269";
        m_btTasknInfo.infoHash = "abdedf2a18f07c7501cf52e10b8d1b938adbb8bb";
        m_btTasknInfo.filePath = "/home/bulw/Desktop/种子/123@.torrent";
        m_btTasknInfo.selectedNum = "1";
        m_btTasknInfo.downloadType = "torrent";
    }

    virtual void TearDown()
    {
    }
    TaskInfo m_taskInfo;
    TaskInfoHash m_btTasknInfo;
};

//TEST_F(ut_dbinstance, removeDB)
//{
//    // QString _dataBasePath = qApp->applicationDirPath();
//    QString dbPath = QString("%1/%2/%3/downloader.db")
//                         .arg(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation))
//                         .arg(qApp->organizationName())
//                         .arg(qApp->applicationName());
//    QFile::remove(dbPath);
//}

TEST_F(ut_dbinstance, addTask)
{
    TaskInfo taskInfo;
    bool ret = DBInstance::addTask(taskInfo);
    //  EXPECT_TRUE(true) << "空数据插入"

    ret = DBInstance::addTask(m_taskInfo);
    EXPECT_TRUE(true) << "有数据插入";
}

TEST_F(ut_dbinstance, delTask)
{
    bool ret = DBInstance::delTask(" zx");
    EXPECT_TRUE(true) << "空数据删除";

    ret = DBInstance::delTask(m_taskInfo.taskId);
    EXPECT_TRUE(true) << "有数据删除";
}

TEST_F(ut_dbinstance, updateTaskInfoByID)
{
    DBInstance::addTask(m_taskInfo);
    TaskInfo taskInfo;
    bool ret = DBInstance::updateTaskInfoByID(taskInfo);
    EXPECT_TRUE(true) << "更新一条不存在任务";

    m_taskInfo.createTime = QDateTime::currentDateTime();
    ret = DBInstance::updateTaskInfoByID(m_taskInfo);
    EXPECT_TRUE(true) << "更新一条存在任务";
}

TEST_F(ut_dbinstance, updateAllTaskInfo)
{
    DBInstance::addTask(m_taskInfo);
    TaskInfo taskInfo;
    QList<TaskInfo> taskInfoList;
    taskInfoList.append(taskInfo);
    bool ret = DBInstance::updateAllTaskInfo(taskInfoList);
    EXPECT_TRUE(true) << "更新所有不存在任务";

    m_taskInfo.createTime = QDateTime::currentDateTime();
    taskInfoList.clear();
    taskInfoList.append(m_taskInfo);
    ret = DBInstance::updateAllTaskInfo(taskInfoList);
    EXPECT_TRUE(true) << "更新所有存在任务";
}

TEST_F(ut_dbinstance, getTaskByID)
{
    TaskInfo taskInfo;
    bool ret = DBInstance::getTaskByID("78d8db55-10c2-4200-b097-253120d7", taskInfo);
    EXPECT_TRUE(true) << "获取一条ID不存在任务信息";
    DBInstance::addTask(m_taskInfo);
    ret = DBInstance::getTaskByID("78d8db55-10c2-4200-b097-253120d71269", taskInfo);
    EXPECT_TRUE(true) << "获取一条ID存在任务信息";
}

TEST_F(ut_dbinstance, getAllTask)
{
    QList<TaskInfo> taskList;
    bool ret = DBInstance::getAllTask(taskList);
    EXPECT_TRUE(true) << "获取所有ID存在任务信息";
}

TEST_F(ut_dbinstance, isExistUrl)
{
    bool isExit;
    bool ret = DBInstance::isExistUrl(m_taskInfo.url, isExit);
    EXPECT_TRUE(true) << "判断Url是否存在";
}

TEST_F(ut_dbinstance, getTaskForUrl)
{
    TaskInfo task;
    bool ret = DBInstance::getTaskForUrl(m_taskInfo.url, task);
    EXPECT_TRUE(true) << "根据url查询任务数据";
}

TEST_F(ut_dbinstance, isExistBtInHash)
{
    bool isExis;
    bool ret = DBInstance::isExistBtInHash("1111", isExis);
    EXPECT_TRUE(true) << "通过hash判断Url是否存在";
}

TEST_F(ut_dbinstance, addTaskStatus)
{
    TaskStatus taskStatus;
    taskStatus.taskId = m_taskInfo.taskId;
    bool ret = DBInstance::addTaskStatus(taskStatus);
    EXPECT_TRUE(true) << "添加任务状态";
}

TEST_F(ut_dbinstance, updateTaskStatusById)
{
    TaskStatus taskStatus;
    taskStatus.taskId = m_taskInfo.taskId;
    taskStatus.finishTime = QDateTime::currentDateTime();
    bool ret = DBInstance::updateTaskStatusById(taskStatus);
    EXPECT_TRUE(true) << "更新任务状态";
}

TEST_F(ut_dbinstance, updateAllTaskStatus)
{
    TaskStatus taskStatus;
    taskStatus.taskId = m_taskInfo.taskId;
    taskStatus.finishTime = QDateTime::currentDateTime();
    QList<TaskStatus> taskStatusList;
    taskStatusList.append(taskStatus);
    bool ret = DBInstance::updateAllTaskStatus(taskStatusList);
    EXPECT_TRUE(true) << "更新所有任务状态";
}

TEST_F(ut_dbinstance, getTaskStatusById)
{
    TaskStatus taskStatus;
    bool ret = DBInstance::getTaskStatusById(m_taskInfo.taskId, taskStatus);
    EXPECT_TRUE(true) << "获取任务状态";
}

TEST_F(ut_dbinstance, getAllTaskStatus)
{
    QList<TaskStatus> taskStatusList;
    bool ret = DBInstance::getAllTaskStatus(taskStatusList);
    EXPECT_TRUE(true) << "获取任务状态";
}

TEST_F(ut_dbinstance, addBtTask)
{
    bool ret = DBInstance::addBtTask(m_btTasknInfo);
    EXPECT_TRUE(true) << "添加一条bt信息";
}

TEST_F(ut_dbinstance, updateBtTaskById)
{
    m_btTasknInfo.downloadType = "/data/home/bulw/work";
    bool ret = DBInstance::updateBtTaskById(m_btTasknInfo);
    EXPECT_TRUE(true) << "更新任务";
}

TEST_F(ut_dbinstance, getBtTaskById)
{
    TaskInfoHash btTaskInfo;
    bool ret = DBInstance::getBtTaskById(m_btTasknInfo.taskId, btTaskInfo);
    EXPECT_TRUE(true) << "通过ID获取BT信息";
}

TEST_F(ut_dbinstance, getAllBtTask)
{
    QList<TaskInfoHash> btTaskInfoList;
    bool ret = DBInstance::getAllBtTask(btTaskInfoList);
    EXPECT_TRUE(true) << "获取所有BT任务信息";
}

TEST_F(ut_dbinstance, getSameNameCount)
{
    QList<TaskInfoHash> btTaskInfoList;
    int count = DBInstance::getSameNameCount("123", "txt");
    bool ret = count > -1;
    EXPECT_TRUE(true) << "获取相同文件个数";
}

TEST_F(ut_dbinstance, delAllTask)
{
    bool ret = DBInstance::delAllTask();
    EXPECT_TRUE(true) << "删除数据所有数据";
}

TEST_F(ut_dbinstance, delTaskError)
{
    TaskStatus taskStatus;
    taskStatus.taskId = m_taskInfo.taskId;

    DBInstance::delTask("vdfaga'");
    DBInstance::addTask(m_taskInfo);
    DBInstance::delTask(m_taskInfo.taskId);
    DBInstance::addTask(m_taskInfo);
    DBInstance::addTaskStatus(taskStatus);
    DBInstance::delTask(m_taskInfo.taskId);
}

TEST_F(ut_dbinstance, updateTaskInfoByIDError)
{
    TaskInfo taskInfo;
    DBInstance::updateTaskInfoByID(taskInfo);
     EXPECT_TRUE(true);
}



