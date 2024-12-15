// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DUPLOADTASKDATA_H
#define DUPLOADTASKDATA_H

#include <QString>
#include <QSharedPointer>
#include <QVector>
#include <QDateTime>

//上传任务相关数据
class DUploadTaskData
{
public:
    //任务类型
    enum TaskType {
        Create, //创建
        Modify, //修改
        Delete, //删除
    };
    //任务对象
    enum TaskObject {
        Task_ScheduleType, //日程类型
        Task_Schedule, //日程
        Task_Color, //类型颜色
    };

    static QString sql_table_name(int task_obj);
    static QString sql_table_primary_key(int task_obj);

    typedef QSharedPointer<DUploadTaskData> Ptr;
    typedef QVector<Ptr> List;
    DUploadTaskData();
    TaskType taskType() const;
    void setTaskType(const TaskType &taskType);

    TaskObject taskObject() const;
    void setTaskObject(const TaskObject &taskObject);

    QString objectId() const;
    void setObjectId(const QString &objectId);

    QString taskID() const;
    void setTaskID(const QString &taskID);

    QDateTime dtCreate() const;
    void setDtCreate(const QDateTime &dtCreate);

private:
    TaskType m_taskType;
    TaskObject m_taskObject;
    QString m_objectId;
    QString m_taskID;
    QDateTime m_dtCreate;
};

#endif // DUPLOADTASKDATA_H
