// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#define log_switch 0
#define dbus_switch 1

#include "../common/utils.h"
#if dbus_switch
#include "types/defenderprocessinfo.h"
#include "types/defenderprocesslist.h"
#endif
#include <QObject>

typedef std::map<int, proc_t> StoredProcType;

struct processStruct {
    QString title;
    QString desktop;
};

//class NetworkProtectionWorker;
class GetPidDetailName : public QObject
{
    Q_OBJECT
public:
    explicit GetPidDetailName();
    ~GetPidDetailName();

    void updateStatus();

    processStruct getDesktopName(const QString &id);
    DefenderProcessInfoList getProcInfoList();
    DefenderProcessList getProcList();
    QList<QString> getGuiProcDesktopList();
    QList<QString> getSingleProcDesktopList();
    QString getProcessEnvironmentVariable(int pid, QString environmentName = "GIO_LAUNCHED_DESKTOP_FILE");
    QString getProcCmdline(int pid);

Q_SIGNALS:
    void reqStartTimer();
    void notifyUpdateGuiProcList();

public Q_SLOTS:
    void onReqStartTimer();
    void GetFileList(QString dir);

private:
    FindWindowTitle *m_findWindowTitle;
    QList<int> m_procList;//用于存储全部有desktop的进程pid
    QList<int> m_guiPids;//存储当前所有打开的GUI窗口的pid
    QList<QString> m_singleProcDesktopList;//仅仅用于显示一个应用的list
    QList<QString> m_guiDesktioList;//用于存储Gui进程的desktop，使用到的地方可以用它做显示判断
    QVector<int> m_abandonProcList;//丢弃的进程，即不会使用到的进程
#if dbus_switch
    DefenderProcessInfoList m_processInfoList;
#endif
};
