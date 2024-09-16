// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
* @file %{CurrentDocument:dbinstance.h}
* @brief 数据库接口库
* @author bulongwei  <bulongwei@uniontech.com>
* @version 1.0.0
* @date %{CurrentDate:2020-05-26} %{CurrentTime:17:59}
* @copyright 2020-%{CurrentDate:2020} Uniontech Technology Co., Ltd.
*/

#ifndef DBINSTANCE_H
#define DBINSTANCE_H
#include "database.h"
#include "dbdefine.h"
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>

class DBInstance
{
public:
    /**
     * @brief 添加任务
     * @param task 任务信息
     * @return true 执行成功 false执行失败
     */
    static bool addTask(TaskInfo &task);
    /**
     * @brief 删除任务
     * @param id 根据id删除数据库中任务
     * @return true 执行成功 false执行失败
     */
    static bool delTask(QString id);
    /**
     * @brief 删除数据库中所有任务
     * @return true 执行成功 false执行失败
     */
    static bool delAllTask();
    /**
     * @brief 更新任务
     * @param task 传入任务，根据类中id更新数据库中任务信息
     * @return true 执行成功 false执行失败
     */
    static bool updateTaskInfoByID(TaskInfo &task);

    /**
     * @brief 更新所有任务
     * @param taskList 传入任务，根据类中id更新数据库中任务信息
     * @return true 执行成功 false执行失败
     */
    static bool updateAllTaskInfo(QList<TaskInfo> &taskList);
    /**
     * @brief 更新任务
     * @param id 需要查询任务id
     * @param task 传出根据id所得任务信息
     * @return true 执行成功 false执行失败
     */
    static bool getTaskByID(QString id, TaskInfo &task);
    /**
     * @brief 获取所有任务
     * @param taskList 传出QList<Task>任务
     * @return true 执行成功 false执行失败
     */
    static bool getAllTask(QList<TaskInfo> &taskList); //得到所有任务

    /**
     * @brief 判断Url是否存在
     * @param url 需要查询url
     * @param ret 传出参数，真为存在，假为不存在
     * @return true 执行成功 false执行失败
     */
    static bool isExistUrl(QString url, bool &ret);

    /**
     * @brief 根据磁力链接获取任务ID
     * @param url 需要查询url
     * @return 任务ID
     */
    static QString getTaskIdByMagnet(QString url);

    /**
     * @brief 根据url查询任务数据
     * @param url 需要查询url
     * @param ret 传出参数，真为存在，假为不存在
     * @return true 执行成功 false执行失败
     */
    static bool getTaskForUrl(QString url, TaskInfo &task);
    /**
     * @brief 判断Url是否存在
     * @param hash 需要查询hash
     * @param ret 传出参数，真为存在，假为不存在
     * @return true 执行成功 false执行失败
     */
    static bool isExistBtInHash(QString hash, bool &ret);
    /**
     * @brief 添加任务状态
     * @param task 任务状态
     * @return true 执行成功 false执行失败
     */
    static bool addTaskStatus(TaskStatus &task); //添加TaskStatus任务状态
    /**
     * @brief 更新任务状态
     * @param @param task 传入任务状态，根据类中id更新数据库中任务状态信息
     * @return true 执行成功 false执行失败
     */
    static bool updateTaskStatusById(TaskStatus &task); //根据id更新TaskStatus任务

    /**
     * @brief 更新所有任务状态
     * @param @param taskList 传入任务状态，根据类中id更新数据库中任务状态信息
     * @return true 执行成功 false执行失败
     */
    static bool updateAllTaskStatus(QList<TaskStatus> &taskList);

    /**
     * @brief 获取任务状态
     * @param id 需要查询任务状态id
     * @param task 传出根据id所得任务状态信息
     * @return true 执行成功 false执行失败
     */
    static bool getTaskStatusById(QString id, TaskStatus &task); //通过id得到任务状态信息

    /**
     * @brief 获取所有任务状态
     * @return true 执行成功 false执行失败
     */
    static bool getAllTaskStatus(QList<TaskStatus> &taskList); //获取所有任务状态信息

    /**
     * @brief 添加url
     * @param url url信息
     * @return true 执行成功 false执行失败
     */
    static bool addBtTask(TaskInfoHash &url); //添加一条bt信息
    /**
     * @brief 更新任务
     * @param id 需要查询任务id
     * @param task 传出根据id所得任务信息
     * @return true 执行成功 false执行失败
     */
    static bool updateBtTaskById(TaskInfoHash &url); //根据id更新bt信息
    /**
     * @brief 获取bt信息
     * @param taskId 任务id
     * @param task 传出根据id所得bt信息
     * @return true 执行成功 false执行失败
     */
    static bool getBtTaskById(QString taskId, TaskInfoHash &url); //根据id得到bt信息
    /**
     * @brief 获取所有url
     * @param urlList 传出所有任务
     * @return true 执行成功 false执行失败
     */
    static bool getAllBtTask(QList<TaskInfoHash> &urlList); //得到所有bt信息

    /**
     * @brief 获取重复名称个数
     * @param filename 文件名称
     * @return 重复名称个数
     */
    static int getSameNameCount(QString filename, QString type);
};

#endif // DBINSTANCE_H
