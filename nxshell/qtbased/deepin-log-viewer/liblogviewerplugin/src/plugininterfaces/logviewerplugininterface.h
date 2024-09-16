// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LOGVIEWERPLUGININTERFACE_H
#define LOGVIEWERPLUGININTERFACE_H

#include "../application/structdef.h"

#include <QtPlugin>

class LogViewerPluginInterface
{  
public:
    LogViewerPluginInterface();
    virtual ~LogViewerPluginInterface();
    //~LogViewerPluginInterface() {}

    /**
     * @brief generateAppFile 获取系统应用日志的数据
     * @param path 要获取的某一个应用的日志的日志文件路径
     * @param id 时间筛选id 对应BUTTONID枚举,0表示全部,1是今天,2是3天内,3是筛选1周内数据,4是筛选一个月内的,5是三个月
     * @param lId 等级筛选id,对应PRIORITY枚举,直接传入获取系统接口,-1表示全部等级不筛选,
     * @param iSearchStr 搜索关键字,现阶段不用,保留参数
     */
    virtual void generateAppFile(const QString &path, BUTTONID id, PRIORITY lId, const QString &iSearchStr = "") = 0;

    // exportAppLogFile 导出应用日志
    // param: path 导出路径
    // param: period 周期，枚举类型，具体值对应周期下拉框选项
    // param: Level 级别，级别，具体值对应级别下拉框选项
    // param: appid 自研应用id，用于导出指定应用的日志，若为空，什么都不做，返回false（例：导出相册的日志信息，传入deepin-album即可）
    // return：true 导出成功， false 导出失败
    virtual bool exportAppLogFile(const QString &path, BUTTONID period, PRIORITY level, const QString &appid) = 0;

    //signals:
    //应用日志数据读取结束信号
    virtual void sigAppFinished(int index) = 0;
    //应用日志数据信号
    virtual void sigAppData(int index, QList<LOG_MSG_APPLICATOIN> iDataList) = 0;

    //public slots:
    //virtual void onSomething(QString txt) = 0;

    //消除警告：has no out-of-line virtual method definitions
    //virtual void nothing();
};

#define LogViewerPluginInterface_iid "com.deepin.logviewer.LogViewerPluginInterface/1.0"
Q_DECLARE_INTERFACE(LogViewerPluginInterface, LogViewerPluginInterface_iid)

#endif // LOGVIEWERPLUGININTERFACE_H
