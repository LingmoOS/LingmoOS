// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CLEANERDBUSADAPTORIMPL_H
#define CLEANERDBUSADAPTORIMPL_H

#include "cleanerdbusadaptorinterface.h"

#include <QObject>

class ComDeepinDefenderDatainterfaceInterface;
class ComDeepinDefenderMonitorNetFlowInterface;
class QGSettings;

class CleanerDBusAdaptorImpl : public QObject
    , public CleanerDBusAdaptorInterface
{
    Q_OBJECT
public:
    explicit CleanerDBusAdaptorImpl(QObject *parent = nullptr);
    virtual ~CleanerDBusAdaptorImpl() {}

    // 后台服务：deepin-defender-datainterface
    virtual QStringList GetTrashInfoList();
    virtual QStringList GetCacheInfoList();
    virtual QStringList GetLogInfoList();
    virtual QStringList GetHistoryInfoList();
    virtual QString GetAppTrashInfoList();
    virtual QString GetBrowserCookiesInfoList();
    virtual void DeleteSpecifiedFiles(QStringList);
    // 安全日志内容写入
    virtual void AddSecurityLog(int nType, const QString &sInfo);
    // Gsetting读写，单元测试不用使用gsetting，所以加到这里做mock
    virtual QString GetValueFromeGSettings(const QString &key);
    virtual void SetValueToGSettings(const QString &key, const QString &value);

private:
    // 后台服务接口
    ComDeepinDefenderDatainterfaceInterface *m_dataInterFaceServer;
    ComDeepinDefenderMonitorNetFlowInterface *m_monitorInterFaceServer;
    QGSettings *m_gsettings;

private:
    // 禁用拷贝构造与赋值
    CleanerDBusAdaptorImpl(const CleanerDBusAdaptorImpl &);
    CleanerDBusAdaptorImpl &operator=(const CleanerDBusAdaptorImpl &);
};

#endif // CLEANERDBUSADAPTORIMPL_H
