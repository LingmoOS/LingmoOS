// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "duploadtaskdata.h"

QString DUploadTaskData::sql_table_name(int task_obj)
{
    switch(task_obj) {
    case DUploadTaskData::Task_ScheduleType:
        return "scheduleType";
    case DUploadTaskData::Task_Schedule:
        return "schedules";
    case DUploadTaskData::Task_Color:
        return "typeColor";
    }
    return "";
}

QString DUploadTaskData::sql_table_primary_key(int task_obj)
{
    switch(task_obj) {
    case DUploadTaskData::Task_ScheduleType:
        return "typeID";
    case DUploadTaskData::Task_Schedule:
        return "scheduleID";
    case DUploadTaskData::Task_Color:
        return "colorID";
    }
    return "";
}

DUploadTaskData::DUploadTaskData()
    : m_taskType(Create)
    , m_taskObject(Task_ScheduleType)
    , m_objectId("")
    , m_taskID("")
{
}

DUploadTaskData::TaskType DUploadTaskData::taskType() const
{
    return m_taskType;
}

void DUploadTaskData::setTaskType(const TaskType &taskType)
{
    m_taskType = taskType;
}

DUploadTaskData::TaskObject DUploadTaskData::taskObject() const
{
    return m_taskObject;
}

void DUploadTaskData::setTaskObject(const TaskObject &taskObject)
{
    m_taskObject = taskObject;
}

QString DUploadTaskData::objectId() const
{
    return m_objectId;
}

void DUploadTaskData::setObjectId(const QString &objectId)
{
    m_objectId = objectId;
}

QString DUploadTaskData::taskID() const
{
    return m_taskID;
}

void DUploadTaskData::setTaskID(const QString &taskID)
{
    m_taskID = taskID;
}

QDateTime DUploadTaskData::dtCreate() const
{
    return m_dtCreate;
}

void DUploadTaskData::setDtCreate(const QDateTime &dtCreate)
{
    m_dtCreate = dtCreate;
}
