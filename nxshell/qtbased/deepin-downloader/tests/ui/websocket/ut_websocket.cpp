// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//#include <QObject>
//#include <QJsonObject>
//#include <iostream>
//#include <QWebSocket>
//#include <QWebSocketServer>

//#include "gtest/gtest.h"
//#include "websockettransport.h"
//#include "websocketclientwrapper.h"
//#include "dbinstance.h"
//#include "database.h"

//class ut_websocket : public ::testing::Test
//{
//protected:
//    ut_websocket()
//    {
//    }

//    virtual ~ut_websocket()
//    {
//    }
//    virtual void SetUp()
//    {
//    }

//    virtual void TearDown()
//    {
//    }
//};

//TEST_F(ut_websocket, WebSocketTransport)
//{
//    QWebSocket *soc = new QWebSocket;
//    WebSocketTransport *port = new WebSocketTransport(soc);
//    QJsonObject message;
//    port->sendMessage(message);
//    port->textMessageReceived("adads");
//    EXPECT_TRUE(true);
//    delete port;
//    delete soc;
//}


//TEST_F(ut_websocket, WebSocketClientWrapper)
//{
//    QWebSocketServer *soc = new QWebSocketServer(QStringLiteral("QWebChannel Server"), QWebSocketServer::NonSecureMode);
//    WebSocketClientWrapper *wrapper = new WebSocketClientWrapper(soc, new QObject);
//    QJsonObject message;
//    wrapper->handleNewConnection();
//    EXPECT_TRUE(true);
//    delete wrapper;
//    delete soc;
//}

//TEST_F(ut_websocket, erroOpenDb)
//{
//    TaskInfo task;
//    TaskStatus taskStatus;
//    TaskInfoHash taskInfoHash;
//    QList<TaskInfo> taskList;
//    QList<TaskStatus> taskStatusList;
//    QList<TaskInfoHash> taskInfoHashList;
//    bool ret = true;
//    DataBase::Instance().m_db = QSqlDatabase::addDatabase("QSQ23LITE");
//    DBInstance::addTask(task);
//    DBInstance::delTask(task.taskId);
//    DBInstance::delAllTask();
//    DBInstance::updateTaskInfoByID(task);
//    DBInstance::updateAllTaskInfo(taskList);
//    DBInstance::getTaskByID("11", task);
//    DBInstance::getAllTask(taskList);
//    DBInstance::isExistUrl("11",ret);
//    DBInstance::addTaskStatus(taskStatus);
//    DBInstance::updateTaskStatusById(taskStatus);
//    DBInstance::updateAllTaskStatus(taskStatusList);
//    DBInstance::getTaskStatusById("11", taskStatus);
//    DBInstance::getAllTaskStatus(taskStatusList);
//    DBInstance::addBtTask(taskInfoHash);
//    DBInstance::updateBtTaskById(taskInfoHash);
//    DBInstance::getBtTaskById("11", taskInfoHash);
//    DBInstance::getAllBtTask(taskInfoHashList);
//    DBInstance::getSameNameCount("11","11");
//    DBInstance::isExistBtInHash("11", ret);
//    DBInstance::getTaskForUrl("11", task);

//    DataBase::Instance().m_db = QSqlDatabase::addDatabase("QSQLITE");
//    DBInstance::addTask(task);
//    DataBase::Instance().destory();
//}


