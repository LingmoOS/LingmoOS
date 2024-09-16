// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "defenderhandlethread.h"

DefenderHandleThread::DefenderHandleThread(TrafficDetailsWorker *worker, QThread *parent)
    : QThread(parent)
    , m_launcherInter(new Launcherd("com.deepin.dde.daemon.Launcher", "/com/deepin/dde/daemon/Launcher", QDBusConnection::sessionBus(), this))
    , m_worker(worker)
    , m_time(new QTimer)
{
    connect(this, &DefenderHandleThread::notifyUpdateAllProcPid, this, &DefenderHandleThread::UpdateAllProcPid);
    connect(m_time, &QTimer::timeout,this,&DefenderHandleThread::onUpdate);//1s更新一次数据
    m_time->start(1000);
    allApp app;
    flowallapp.clear();//清除系统所有应用信息列表
    QDBusPendingReply<LauncherItemInfoList> m_reply;
    m_reply = m_launcherInter->GetAllItemInfos();
    m_reply.waitForFinished();
    if (m_reply.isError()) {
        qWarning() << "data is empty, quit!!";
        qWarning() << m_reply.error();
        qApp->quit();
    }
    const LauncherItemInfoList &datas = m_reply.value();
    for (auto at : datas) {
        app.Name = at.Name;
        app.Icon = at.Icon;
        app.ID = at.ID;
        app.Path = at.Path;
        allapp << app;
        flowallapp << app;
#if Work_Launcher_Flag
        qDebug() << at.ID << at.Icon;
#endif
    }
    //添加http到系统所有应用列表中
    app.Name = "https";
    app.ID = "https";
    app.Icon = ":/imageresource/dcc_application_executable.svg";
    app.Path = "https";
    allapp << app;

    m_flowRangeStyle = FlowRangeStyle::today;
    Q_EMIT notifyUpdateAllProcPid();//有应用更新
    //获取数据
}

void DefenderHandleThread::run()
{
}

//把网速/流量列表中的上行、下行数据替换成新的
void DefenderHandleThread::getdisplaylist(QList<AppParameter> speedlist, QList<AppParameter> dbflowlist)
{
    static int time = 0;
    QString sName = "";
    QString sIcon = "";
    QString sId = "";
    QString sPath = "";
    double totalspeed = 0;
    //遍历所有应用
    //当天和当月流量需要实时更新
    setFlowName();//绑定flowlist的pidname和flowlist的Sname
    {
        DEFENDER_MUTEX_LOCKER(&SMutex::getInstance().m_mutex);
        for (int i = 0; i < m_displaylist.size(); i++) {
            //遍历网速list
            for (auto &at : speedlist) {
                if ((m_displaylist[i].pidname.contains(at.pidname)) && !at.pidname.isEmpty()) { //名称包含替换
                    m_displaylist[i].download = QString::number(at.recv, 'f', 2);
                    m_displaylist[i].update = QString::number(at.send, 'f', 2);
                    m_displaylist[i].total = QString::number(at.apptotal, 'f', 2);
                    m_displaylist[i].pid = at.pid;
                    m_displaylist[i].pidname = at.pidname;
                    m_displaylist[i].totalupdate += at.send;
                    m_displaylist[i].totaldownload += at.recv;
                }
            }
        }
    }

    {
        //遍历流量list
        DEFENDER_MUTEX_LOCKER(&SMutex::getInstance().m_mutex);
        for (const auto &at : dbflowlist) {
            for (auto it : flowallapp) {
                sName = it.Name;
                sIcon = it.Icon;
                sId = it.ID;
                sPath = it.Path;
                m_showflow.name = sName;
                m_showflow.icon = sIcon;
                m_showflow.path = sPath;
                m_showflow.download = "0";
                m_showflow.update = "0";
                m_showflow.total = "0";
                m_showflow.totalupdate = 0;
                m_showflow.totaldownload = 0;
                m_showflow.totalflow = 0;
                m_showflow.status = 1;
                m_showflow.pidname = "";
                m_showflow.pid = 0;
                if ((sPath.contains(at.pidname) && !at.pidname.isEmpty())) { //名称包含重新赋值
                    m_showflow.download = QString::number(at.recv, 'f', 2);
                    m_showflow.update = QString::number(at.send, 'f', 2);
                    totalspeed = at.recv + at.send;
                    m_showflow.total = QString::number(totalspeed, 'f', 2); //单个APP总流量
                    m_showflow.pidname = at.pidname;
                    m_showflow.name = sName;
                    m_showflow.pid = at.pid;
                    m_showflow.status = 0;
                    m_showflow.totalupdate += at.send;
                    m_showflow.totaldownload += at.recv;
                    m_displayflowlist << m_showflow;
                }
            }
        }
    }
    //清空网速list
    m_worker->resetspeedlist();
    m_stabl_displaylist = m_displayflowlist;
    //发送数据更新信号
    Q_EMIT SendFlowData(m_displayflowlist);
    Q_EMIT SendSpeedData(m_displaylist);
    //清空显示网速和流量list
    m_displayflowlist.clear();
    time++;
}

//设置当前日期
void DefenderHandleThread::setFlowRangeStyle(FlowRangeStyle style)
{
    m_flowRangeStyle = style;
}

//设置flowlist的sname
void DefenderHandleThread::setFlowName()
{
    {
        DEFENDER_MUTEX_LOCKER(&SMutex::getInstance().m_mutex);
        m_flownamelist = m_flowlist;
        for (auto at : flowallapp) {
            for (int i = 0; i < m_flownamelist.size(); i++) {
                if ((at.Path.contains(m_flownamelist.at(i).pidname) || !at.Name.compare(m_flownamelist.at(i).sName)) && !m_flownamelist.at(i).pidname.isEmpty()) {
                    m_flownamelist[i].sName = at.Name;
                }
            }
        }
    }
    Q_EMIT setSaveFlowList(m_flownamelist);
}

//1s更新一次数据
void DefenderHandleThread::onUpdate()
{
    DEFENDER_MUTEX_LOCKER(&SMutex::getInstance().m_mutex);
    m_speedlist = m_worker->GetSpeedList();
    m_flowlist = m_worker->GetFlowFlit();
//    m_dbflowlist = m_worker->Getonshowlist();
    getdisplaylist(m_speedlist, m_dbflowlist);
}

//应用列表更新
void DefenderHandleThread::onUpdateLauncherList()
{
    qDebug()<<"onUpdateLauncherList";
    allApp app;
    allapp.clear();
    flowallapp.clear();
    QDBusPendingReply<LauncherItemInfoList> m_reply;
    m_reply = m_launcherInter->GetAllItemInfos();
    m_reply.waitForFinished();
    if (m_reply.isError()) {
        qWarning() << "data is empty, quit!!";
        qWarning() << m_reply.error();
        qApp->quit();
    }
    const LauncherItemInfoList &datas = m_reply.value();
    for (auto at : datas) {
        app.Name = at.Name;
        app.Icon = at.Icon;
        app.ID = at.ID;
        app.Path = at.Path;
        allapp << app;
        flowallapp << app;
    }

    app.Name = "https";
    app.ID = "https";
    app.Icon = ":/imageresource/dcc_application_executable.svg";
    app.Path = "https";
    allapp << app;
}
//有新应用打开/关闭
void DefenderHandleThread::UpdateAllProcPid()
{
    m_displaylist.clear();
    QString sName = "";
    QString sIcon = "";
    QString sId = "";
    QString sPath = "";
    QString desktop = "";
    {
        DEFENDER_MUTEX_LOCKER(&SMutex::getInstance().m_mutex);
        for (auto it : allapp) {
            sName = it.Name;
            sIcon = it.Icon;
            sId = it.ID;
            sPath = it.Path;
            m_showspeed.name = sName;
            m_showspeed.icon = sIcon;
            m_showspeed.path = sPath;
            m_showspeed.download = "0.00";
            m_showspeed.update = "0.00";
            m_showspeed.total = "0.00";
            m_showflow.totalupdate = 0;
            m_showflow.totaldownload = 0;
            m_showflow.totalflow = 0;
            m_showspeed.status = 1;
            m_showflow.name = sName;
            m_showflow.icon = sIcon;
            m_showflow.path = sPath;
            m_showflow.status = 1;
            m_showflow.pidname = "";
            m_showflow.pid = 0;

            //遍历desktoplist
//            qDebug()<<"getprocList.size = "<<m_worker->GetDesktop().size();
            for (auto &at : m_worker->GetDesktop()) {
                desktop = at;
                desktop = desktop.split("/").last();
                desktop = getSystemDesktop(desktop); // 获取需要特殊转换桌面文件列表中对应的桌面文件
//                qDebug() << "desktop = " << desktop;
                if (sPath.contains(desktop)) {
                    m_showspeed.pidname = desktop;
                    m_showspeed.download = QString::number(0.00, 'f', 2);
                    m_showspeed.update = QString::number(0.00, 'f', 2);
                    m_showspeed.total = QString::number(0.00, 'f', 2);
                    m_showspeed.status = 0;
                    m_showspeed.totalupdate = 0.00;
                    m_showspeed.totaldownload = 0.00;
                    m_displaylist << m_showspeed;
                }
            }
        }
    }
    getdisplaylist(m_speedlist, m_dbflowlist);
}
