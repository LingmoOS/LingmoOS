// SPDX-FileCopyrightText: 2015 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PROCESSINFOMANAGER_H
#define PROCESSINFOMANAGER_H

#include <QObject>
#include <QList>
#include <QTimer>
#include <QPointer>

#include "org_deepin_dde_startmanager1.h"
#include "com_deepin_chromeextension_tabslimit.h"

QString genericAppName(const QString &desktop);

using StartManagerInter = org::deepin::dde::StartManager1;
using ChromeTabsInter = com::deepin::chromeextension::TabsLimit;

class ProcessInfo
{
public:
    int id;
    unsigned total_mem_bytes;
    QString cgroup_path;
    QString app_name;
    QString desktop;
    QStringList pid_list;
    QList<ProcessInfo> sub_procs;
};

class ProcessInfoModel;
class ProcessInfoManager : public QObject
{
    Q_OBJECT

    friend class ProcessInfoModel;

public:
    explicit ProcessInfoManager(QObject *parent = nullptr);

signals:
    void processInfoListChanged() const;

private slots:
    void scanChromeTabs();
    void scanProcessInfos();
    void startRefreshData();
    void stopRefreshData() { m_refreshTimer->stop(); }
    void mergeLists();

    void scanChromeTabsCB(QDBusPendingCallWatcher *watcher);

private:
    void appendCGroupPath(const QString &path, const QString &desktop);
//    void appendChromeTab(const ChromeTabInfo &tabInfo);

private:
    QPointer<QTimer> m_refreshTimer;
    QPointer<StartManagerInter> m_startManagerInter;
    QPointer<ChromeTabsInter> m_chromeTabsInter;
    QString m_sessionId;
    QList<ProcessInfo> tabsInfoList;
    QList<ProcessInfo> processInfoList;
};

#endif // PROCESSINFOMANAGER_H
