// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "zcupsmonitor.h"
#include "zjobmanager.h"
#include "cupsattrnames.h"
#include "common.h"
#include "cupsconnection.h"
#include "cupsconnectionfactory.h"
#include "qtconvert.h"
#include "zsettings.h"
#include "cupsattrnames.h"

#include <DApplication>
#include <DNotifySender>

#include <QMap>
#include <QVariant>
#include <QStringList>
#include <QDebug>
#include <QProcess>
#include <QDBusConnection>
#include <QDBusPendingReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDateTime>
#include <QFile>

#include <algorithm>
#include <regex>

#include <pwd.h>
DWIDGET_USE_NAMESPACE

#define SUB_URI "/"
#define USERDATA "dde-printer"
#define IDLEEXIT 1000 * 60 * 5
#define PROCESSINGTIP 1000 * 60

DCORE_USE_NAMESPACE

static vector<string> g_subEvents = {"printer-deleted", "printer-added", "printer-state-changed", "job-progress", "job-state-changed"};

static bool isLocalUserJob(int jobId)
{
    map<string, string> jobInfo;
    QString jobOriginName;
    if (g_jobManager->getJobById(jobInfo, jobId, 0) == 0) {
        map<string, string>::const_iterator itjob;
        for (itjob = jobInfo.begin(); itjob != jobInfo.end(); ++itjob) {
            if (itjob->first == "job-originating-user-name") {
                jobOriginName = attrValueToQString(itjob->second);
                break;
            }
        }
    }

    struct passwd* pwd = getpwuid(getuid());
    if (pwd != nullptr && (strcmp(jobOriginName.toStdString().c_str(), pwd->pw_name) != 0)) {
        return false; // 如果打印任务名称与本地用户名不一致, 不处理通知
    }
    return true;
}

static QString getPrinterInfo(const QString &strName)
{
    QString strMakeAndModel;

    try {
        vector<string> requestAttrs;
        requestAttrs.push_back("printer-make-and-model");
        auto conPtr = CupsConnectionFactory::createConnectionBySettings();
        if (!conPtr) {
            return strMakeAndModel;
        }
        map<string, string> attrs = conPtr->getPrinterAttributes(strName.toStdString().c_str(), nullptr, &requestAttrs);

        for (auto iter = attrs.begin(); iter != attrs.end(); ++iter) {
            strMakeAndModel = QString::fromStdString(iter->second.data());
            strMakeAndModel = strMakeAndModel.remove(0, 1);
        }
    } catch (const std::runtime_error &e) {
        qCWarning(COMMONMOUDLE) << "Report execpt: " << QString::fromUtf8(e.what());
        strMakeAndModel.clear();
    }

    return strMakeAndModel;
}

static QString getPpdInfo(const QString &strPpdName, const QString &info)
{
    QString strOut, strErr;
    QString commond;
    commond += "cat " + strPpdName;
    if (shellCmd(commond, strOut, strErr) == 0) {
        QStringList list = strOut.split("\n", QString::SkipEmptyParts);
        int index = 0;
        for (; index < list.size(); index++) {
            if (list[index].contains(info))
            break;
        }
        if (index == list.size()) {
            return nullptr;
        }

        QStringList val = list[index].split(":", QString::SkipEmptyParts);
        if (val.count() <= 1) {
            return nullptr;
        }
        return val[1];
    }

    return nullptr;
}

static QString getPackageVerByName(const QString &packageName)
{
    QString strOut, strErr;
    if (shellCmd(QString("dpkg -l %1").arg(packageName), strOut, strErr) == 0) {
        QStringList list = strOut.split("\n", QString::SkipEmptyParts);
        strOut = list.isEmpty() ? "" : list.last();
        list = strOut.split(" ", QString::SkipEmptyParts);
        return list.count() > 2 ? list[2] : QString();
    }

    return nullptr;
}

static QString formatDateTime(const QString &time)
{
    QDateTime timetemp = QDateTime::fromSecsSinceEpoch(static_cast<uint>(time.toInt()));
    return timetemp.toString("yyyy-MM-dd hh:mm:ss");
}

static QString replaceChineseWithAsterisk(const QString &text)
{
    QString result = text;

    for (int i = 0; i < result.length(); ++i) {
        QChar character = result[i];
        if (character >= 0x4e00 && character <= 0x9fff) {
            result.replace(i, 1, "*");
        }
    }

    return result;
}

CupsMonitor::CupsMonitor(QObject *parent)
    : QThread(parent)
    , m_jobId(0)
    , m_bQuit(false)
{
    m_subId = -1;
    m_seqNumber = -1;
}

CupsMonitor::~CupsMonitor()
{
    stop();
    unloadEventLib();
}

void CupsMonitor::initTranslations()
{
    if (m_stateStrings.isEmpty()) {
        m_stateStrings.append("");
        m_stateStrings.append("");
        m_stateStrings.append("");
        m_stateStrings.append(tr("Queuing"));
        m_stateStrings.append(tr("Paused"));
        m_stateStrings.append(tr("Printing"));
        m_stateStrings.append(tr("Stopped"));
        m_stateStrings.append(tr("Canceled"));
        m_stateStrings.append(tr("Error"));
        m_stateStrings.append(tr("Completed"));
    }
}

QString CupsMonitor::getStateString(int iState)
{
    return iState < m_stateStrings.count() ? m_stateStrings[iState] : QString();
}

void CupsMonitor::run()
{
    qCInfo(COMMONMOUDLE) << "Task cupsmonitor running...";
    int iRet = 0;

    iRet = doWork();

    qCInfo(COMMONMOUDLE) << "Task cupsmonitor finished " << iRet;

}

bool CupsMonitor::isCompletedState(int state)
{
    return (IPP_JSTATE_COMPLETED == state || IPP_JSTATE_ABORTED == state || IPP_JSTATE_CANCELED == state);
}

bool CupsMonitor::insertJobMessage(int id, int state, const QString &message)
{
    QString str;
    int times = 0;
    bool hasRuningJobs = false;

    {
        QMutexLocker locker(&m_mutex);

        //获取对应任务状态更新的次数，如果大于5次重置保存的次数，通知界面刷新一次
        str = m_jobMessages.value(id);
        if (!str.isEmpty()) {
            str = str.split(" ").first();

            if (str.startsWith(QString::number(state))) {
                times = str.mid(1).toInt();
                times = times > 5 ? 0 : times;
            }
        }
        times++;

        str = QString::number(state) + QString::number(times) + " " + message;
        m_jobMessages.insert(id, str);

        QStringList msglist = m_jobMessages.values();
        foreach (str, msglist) {
            if (!str.isEmpty()) {
                int iState = str.left(1).toInt();

                if (!isCompletedState(iState)) {
                    hasRuningJobs = true;
                    break;
                }
            }
        }
    }
    emit signalShowTrayIcon(hasRuningJobs);

    //只有处理中的状态才通过事件触发的次数过滤事件
    if (IPP_JSTATE_PROCESSING != state) {
        times = 1;
    }

    return times == 1;
}

bool CupsMonitor::isJobPurged(int id)
{
    QString str = getJobMessage(id);

    return str.indexOf("Job purged") >= 0;
}

QString CupsMonitor::getJobMessage(int id)
{
    QString str;
    QStringList list;

    {
        QMutexLocker locker(&m_mutex);
        str = m_jobMessages.value(id);
    }

    if (!str.isEmpty()) {
        list = str.split(" ");
        list.removeFirst();
        str = list.join(" ");
    }

    return str;
}

QString CupsMonitor::getJobNotify(const QMap<QString, QVariant> &job)
{
    int iState = job[JOB_ATTR_STATE].toString().toInt();
    QString strState = m_stateStrings[iState];
    int id = job[JOB_ATTR_ID].toInt();

    if (IPP_JSTATE_ABORTED == iState || IPP_JSTATE_PROCESSING == iState || IPP_JSTATE_STOPPED == iState) {
        QString jobmessage = getJobMessage(id);
        QString printermessage = job[JOB_ATTR_STATE_MEG].toString();
        if (!jobmessage.isEmpty()) {
            strState += QString(" [%1]").arg(jobmessage);
        } else if (!printermessage.isEmpty()) {
            strState += QString(" [%1]").arg(printermessage);
        }
    }

    return strState;
}

void CupsMonitor::clearSubscriptions()
{
    try {
        auto conPtr = CupsConnectionFactory::createConnectionBySettings();
        if (!conPtr)
            return;
        vector<map<string, string>> subs = conPtr->getSubscriptions(SUB_URI, true, -1);
        for (size_t i = 0; i < subs.size(); i++) {
            dumpStdMapValue(subs[i]);
            m_subId = attrValueToQString(subs[i]["notify-subscription-id"]).toInt();
            conPtr->cancelSubscription(m_subId);
        }
    } catch (const std::exception &ex) {
        qCWarning(COMMONMOUDLE) << "Got execpt: " << QString::fromUtf8(ex.what());
        return;
    }
}

int CupsMonitor::createSubscription()
{
    /*
     * 不能直接清空订阅，里面还有system-config-printer的订阅信息
     * 采用notify-user-data来区分是哪个应用创建的订阅，data设置为dde-printer
     * 对比获取的订阅events，然后比较当前订阅events，如果当前events存在上次订阅events没有的event，那么就更新订阅（这里不能简单比较订阅events的数量来判断）
    */
    m_subId = g_Settings->getSubscriptionId();
    auto conPtr = CupsConnectionFactory::createConnectionBySettings();
    if (!conPtr) {
        m_subId = -1;
        return m_subId;
    }
    try {
        if (-1 != m_subId) {
            bool isExists = false;
            vector<map<string, string>> subs = conPtr->getSubscriptions(SUB_URI, true, -1);
            for (size_t i = 0; i < subs.size(); i++) {
                dumpStdMapValue(subs[i]);

                if (m_subId == attrValueToQString(subs[i]["notify-subscription-id"]).toInt()) {
                    auto it = subs[i].find("notify-user-data");
                    if (it != subs[i].end()) {
                        QString userData = QString::fromStdString(subs[i]["notify-user-data"].substr(1));
                        userData.resize(strlen(USERDATA));
                        vector<string> events;
                        parseSubEvents(subs[i]["notify-events"], events);
                        if (!userData.compare("dde-printer") && !isNeedUpdateSubscription(events)) {
                            qCInfo(COMMONMOUDLE) << "Use last subscription id: " << m_subId;
                            isExists = true;
                        } else {
                            qCInfo(COMMONMOUDLE) << "Cancel last subscription id: " << m_subId;
                            cancelSubscription();
                            isExists = false;
                        }
                        break;
                    }

                }
            }

            if (!isExists)
                m_subId = -1;
        }
    } catch (const std::exception &ex) {
        qCWarning(COMMONMOUDLE) << "Got execpt: " << QString::fromUtf8(ex.what());
        m_subId = -1;
    }

    try {
        if (-1 == m_subId) {
            m_subId = conPtr->createSubscription(SUB_URI, &g_subEvents, 0, nullptr, 86400, 0, USERDATA);
            g_Settings->setSubscriptionId(m_subId);
            g_Settings->setSequenceNumber(0);
            qCDebug(COMMONMOUDLE) << "createSubscription id: " << m_subId;
        }
    } catch (const std::exception &ex) {
        qCWarning(COMMONMOUDLE) << "Got execpt: " << QString::fromUtf8(ex.what());
        m_subId = -1;
    }

    return m_subId;
}

int CupsMonitor::getNotifications(int &notifysSize)
{
    if (-1 == m_subId)
        return -1;

    try {
        bool skip = m_jobId > 0;
        auto conPtr = CupsConnectionFactory::createConnectionBySettings();
        if (!conPtr)
            return -1;
        vector<map<string, string>> notifys = conPtr->getNotifications(m_subId, m_seqNumber, nullptr, nullptr);
        notifysSize = notifys.size();

        if (notifysSize)
            qCDebug(COMMONMOUDLE) << "Got number:" << notifysSize << "after sequence:" << m_seqNumber;

        for (int i = 0; i < notifysSize; i++) {
            map<string, string> info = notifys.at(i);
            int number = attrValueToQString(info[CUPS_NOTIY_SEQ_NUM]).toInt();
            QString strevent = attrValueToQString(info[CUPS_NOTIY_EVENT]);

            qCDebug(COMMONMOUDLE) << "******************************************************";

            dumpStdMapValue(info);

            if (number >= m_seqNumber) {
                m_seqNumber = number + 1;
                g_Settings->setSequenceNumber(m_seqNumber);
            }

            if (strevent.startsWith("job-")) {
                int iState = attrValueToQString(info[JOB_ATTR_STATE]).toInt();
                int iJob = attrValueToQString(info[CUPS_NOTIY_JOBID]).toInt();
                QString strReason = attrValueToQString(info[CUPS_NOTIY_TEXT]);
                QStringList list = attrValueToQString(info[JOB_ATTR_NAME]).split("/", QString::SkipEmptyParts);
                QString strJobName = list.isEmpty() ? "" : list.last();
                QString strRecordReason;
                qCDebug(COMMONMOUDLE) << "Got a job event: " << iJob << iState << strReason;

                if (iJob == m_jobId) {
                    skip = false;
                    m_jobId = 0;
                }

                if (skip)
                    continue;

                /* 过滤通知消息处理:
                 * 1. 由于getNotifications没有origin name字段, jobid信息notify-job-id;
                 * 2. 通过jobid获取job-originating-user-name字段
                 * 3. job-originating-user-name与username不一致时, 结束通知处理
                 */
                if (!isLocalUserJob(iJob)) { // 非本地用户作业任务，不处理
                    continue;
                }

                //通过判断同一个id，同一个状态插入的次数判断是否触发信号
                if (insertJobMessage(iJob, iState, strReason)) {
                    qCDebug(COMMONMOUDLE) << "Emit job state changed signal" << iJob << iState << strReason;
                    emit signalJobStateChanged(iJob, iState, strReason);
                }

                switch (iState) {
                case IPP_JSTATE_PROCESSING:
                    if (m_processingJob.contains(iJob)) {
                        const QTime &t = m_processingJob[iJob];
                        if (!t.isNull() && t.elapsed() > PROCESSINGTIP) {
                            strReason = tr("%1 timed out, reason: %2").arg(strJobName).arg(strReason);
                            sendDesktopNotification(0, qApp->productName(), strReason, 3000);
                            m_processingJob[iJob] = QTime();
                        }
                    } else {
                        QTime t;
                        t.start();
                        m_processingJob.insert(iJob, t);
                    }
                    break;
                case IPP_JSTATE_COMPLETED:
                case IPP_JSTATE_STOPPED:
                case IPP_JSTATE_ABORTED: {
                    if (IPP_JSTATE_COMPLETED == iState) {
                        strReason = tr("%1 has been sent to the printer, please take away the paper in time!").arg(strJobName);
                    } else {
                        strReason = tr("%1 %2, reason: %3")
                                    .arg(strJobName)
                                    .arg(getStateString(iState).toLower())
                                    .arg(strReason);
                    }
                    sendDesktopNotification(0, qApp->productName(), strReason, 3000);

                    Q_FALLTHROUGH();
                }
                case IPP_JOB_CANCELLED:
                    m_processingJob.remove(iJob);

                    strRecordReason = strReason.replace(strJobName, "Printing File");
                    writeJobLog(iState == IPP_JSTATE_COMPLETED, iJob, strRecordReason);
                    break;
                default:
                    break;
                }
            } else {
                if (skip)
                    continue;

                if ("printer-state-changed" == strevent) {
                    int iState = attrValueToQString(info[CUPS_OP_STATE]).toInt();
                    QString printerName = attrValueToQString(info[CUPS_OP_NAME]);
                    QString strReason = attrValueToQString(info[CUPS_OP_STATIE_RES]);

                    if ("none" == strReason)
                        strReason = attrValueToQString(info[CUPS_NOTIY_TEXT]);

                    qCDebug(COMMONMOUDLE) << "Printer state changed: " << printerName << iState << strReason;

                    //只有状态改变的时候才触发信号
                    if (m_printersState.value(printerName, -1) != iState) {
                        qCDebug(COMMONMOUDLE) << "Emit printer state changed signal: " << printerName << iState << strReason;
                        m_printersState.insert(printerName, iState);
                        emit signalPrinterStateChanged(printerName, iState, strReason);

                    }
                } else if ("printer-deleted" == strevent) {
                    QString printerName = attrValueToQString(info[CUPS_OP_NAME]);
                    qCInfo(COMMONMOUDLE) << "signalPrinterDelete";
                    emit signalPrinterDelete(printerName);
                } else if ("printer-added" == strevent) {
                    QString printerName = attrValueToQString(info[CUPS_OP_NAME]);
                    qCInfo(COMMONMOUDLE) << "signalPrinterAdd";
                    emit signalPrinterAdd(printerName);
                }

            }
        }

        if (notifysSize)
            qCDebug(COMMONMOUDLE) << "------------------------------------------------------------";
    } catch (const std::exception &ex) {
        qCWarning(COMMONMOUDLE) << "Got execpt: " << QString::fromUtf8(ex.what());
        return -1;
    }

    return 0;
}

int CupsMonitor::cancelSubscription()
{
    try {
        if (-1 != m_subId) {
            auto conPtr = CupsConnectionFactory::createConnectionBySettings();
            if (conPtr)
                conPtr->cancelSubscription(m_subId);
        }

    } catch (const std::exception &ex) {
        qCWarning(COMMONMOUDLE) << "Got execpt: " << QString::fromUtf8(ex.what());
        return -1;
    }

    return 0;
}

void CupsMonitor::writeJobLog(bool isSuccess, int jobId, QString strReason)
{
    if (!isEventSdkInit()) {
        loadEventlib();
    }

    pfWriteEventLog WriteEventLog = getWriteEventLog();
    if (!WriteEventLog) {
        return;
    }

    QJsonObject obj;
    obj.insert("status", isSuccess ? "Success" : "Abort");
    QString strJobCreateTime, strJobEndTime, strJobUri;
    QStringList strExtmsg;
    map<string, string> jobInfo;
    if (g_jobManager->getJobById(jobInfo, jobId, 0) == 0) {
        map<string, string>::const_iterator itjob;
        for (itjob = jobInfo.begin(); itjob != jobInfo.end(); ++itjob) {
            if (itjob->first == "time-at-completed") {
                strJobEndTime = attrValueToQString(itjob->second);
            } else if (itjob->first == "job-printer-uri") {
                strJobUri = attrValueToQString(itjob->second);
            } else if (itjob->first == JOB_ATTR_TIME_ADD) {
                strJobCreateTime = attrValueToQString(itjob->second);
            } else if (itjob->first == JOB_ATTR_STATE_MEG || itjob->first == JOB_ATTR_STATE_STR) {
                strExtmsg << attrValueToQString(itjob->second);
            }
        }
    }

    QString printerInfo = QString::fromUtf8(QByteArray::fromPercentEncoding(strJobUri.toUtf8()));
    strJobCreateTime = formatDateTime(strJobCreateTime);
    obj.insert("reason", isSuccess ? "" : strReason + strExtmsg.join(" "));
    strJobEndTime = formatDateTime(strJobEndTime);
    obj.insert("jobCreateTime", strJobCreateTime);
    obj.insert("jobEndTime", strJobEndTime);
    obj.insert("printerInfo", replaceChineseWithAsterisk(printerInfo));

    QString jobPrinterName = getPrinterNameFromUri(strJobUri);
    QString  strFilePath = getPrinterPPD(jobPrinterName.toStdString().c_str());
    QString pddPath = QFile::symLinkTarget(strFilePath);
    QString driverInfo = getPrinterInfo(jobPrinterName);

    obj.insert("printerName", replaceChineseWithAsterisk(jobPrinterName));
    obj.insert("driverInfo", driverInfo);

    if (driverInfo.contains("Bisheng")) {
        QString packageName = getPpdInfo(pddPath, "PackageName").replace(QRegExp("^ "), "").replace(QRegExp("\""), "");

        // 获取包版本信息
        QString ver = getPackageVerByName(packageName);
        obj.insert("packageName", packageName);
        obj.insert("packageVer", ver);
        obj.insert("driverType", "bisheng");
    } else {
        obj.insert("driverType", "other");
    }
    obj.insert("version", getPackageVerByName(APPNAME));
    obj.insert("cupsVersion", getPackageVerByName("cups"));
    obj.insert("tid", 1000100001); // 事件ID

    QString logInfo = QString(QJsonDocument(obj).toJson(QJsonDocument::Compact));
    qCDebug(COMMONMOUDLE) << "json data: " << logInfo;
    WriteEventLog(logInfo.toStdString());
}

int CupsMonitor::initSubscription()
{
    if (-1 != m_subId)
        return 0;

    // 不要移动 m_subId 和 m_seqNumber 初始化的顺序
    m_subId = createSubscription();
    m_seqNumber = g_Settings->getSequenceNumber();

    qCInfo(COMMONMOUDLE) << QString("subscription: %1, sequence: %2").arg(m_subId).arg(m_seqNumber);
    if (-1 == m_subId) {
        qCWarning(COMMONMOUDLE) << "Invaild subcription id";
        return -1;
    }

    return 0;
}

int CupsMonitor::resetSubscription()
{
    cancelSubscription();
    m_subId = m_seqNumber = -1;

    return initSubscription();
}

int CupsMonitor::doWork()
{

    m_pendingNotification.clear();
    m_processingJob.clear();

    QTime t;
    t.start();
    while (!m_bQuit) {
        int size = 0;
        if (0 != getNotifications(size) && 0 != resetSubscription()) {
            break;
        }

        m_jobId = 0;

        if (size > 0)
            t.restart();

        if (t.elapsed() > IDLEEXIT)
            break;

        sleep(1);
    }

    return 0;
}

void CupsMonitor::stop()
{
    cancelSubscription();
    m_bQuit = true;
    if (this->isRunning()) {
        this->quit();
        this->wait();
    }
}

int CupsMonitor::getPrinterState(const QString &printer)
{
    return m_printersState.value(printer, -1);
}

bool CupsMonitor::initWatcher()
{
    QDBusConnection conn = QDBusConnection::systemBus();
    /*关联系统的打印队列，当线程退出的时候，如果有新的打印队列，重新唤醒线程*/
    bool success = conn.connect("", "/com/redhat/PrinterSpooler", "com.redhat.PrinterSpooler", "", this, SLOT(spoolerEvent(QDBusMessage)));

    //启动前获取未完成的任务列表，防止遗漏没监听到的任务
    map<int, map<string, string>> jobs;
    if (0 == g_jobManager->getJobs(jobs, WHICH_JOB_RUNING, 1)) {
        map<int, map<string, string>>::iterator itJobs;

        if (jobs.size() > 0) {
            for (itJobs = jobs.begin(); itJobs != jobs.end(); itJobs++) {
                map<string, string> info = itJobs->second;
                int iState = attrValueToQString(info[JOB_ATTR_STATE]).toInt();
                QString message = attrValueToQString(info[JOB_ATTR_STATE_MEG]);
                insertJobMessage(itJobs->first, iState, message);
            }
            start();
        }
    }

    if (!success) {
        qCInfo(COMMONMOUDLE) << "failed to connect spooler dbus";
    }
    return success;
}

int CupsMonitor::sendDesktopNotification(int replaceId, const QString &summary, const QString &body, int expired)
{
    int ret = 0;

    QDBusPendingReply<unsigned int> reply = DUtil::DNotifySender(summary)
                                            .appName("dde-printer")
                                            .appIcon(":/images/dde-printer.svg")
                                            .appBody(body)
                                            .replaceId(replaceId)
                                            .timeOut(expired)
                                            .actions(QStringList() << "default")
                                            .call();

    reply.waitForFinished();
    if (!reply.isError())
        ret = reply.value();

    if (ret > 0)
        m_pendingNotification.insert(ret);

    QDBusConnection conn = QDBusConnection::sessionBus();
    conn.connect("", "/org/freedesktop/Notifications", "org.freedesktop.Notifications", "ActionInvoked", this, SLOT(notificationInvoke(unsigned int, QString)));
    conn.connect("", "/org/freedesktop/Notifications", "org.freedesktop.Notifications", "NotificationClosed", this, SLOT(notificationClosed(unsigned int, unsigned int)));

    return ret;
}

void CupsMonitor::notificationInvoke(unsigned int notificationId, QString action)
{
    Q_UNUSED(action);
    if (m_pendingNotification.contains(notificationId)) {
        showJobsWindow();
    }

}

void CupsMonitor::notificationClosed(unsigned int notificationId, unsigned int reason)
{
    Q_UNUSED(reason);
    m_pendingNotification.remove(notificationId);
}

void CupsMonitor::spoolerEvent(QDBusMessage msg)
{
    QList<QVariant> args = msg.arguments();
    qCDebug(COMMONMOUDLE) << args;

    if (!isRunning()) {
        if (args.size() == 3)
            m_jobId = args[1].toInt();

        if (!isLocalUserJob(m_jobId)) {
            return;
        }
        start();
    }
}

void CupsMonitor::showJobsWindow()
{
    QProcess process;
    QString cmd = "dde-printer";
    QStringList args;
    args << "-m" << "4";
    if (!process.startDetached(cmd, args)) {
        qCWarning(COMMONMOUDLE) << QString("showJobsWindow failed because %1").arg(process.errorString());
    }
}

bool CupsMonitor::isNeedUpdateSubscription(vector<string> &events)
{
    bool ret = false;
    std::sort(events.begin(), events.end());
    std::sort(g_subEvents.begin(), g_subEvents.end());
    ret = std::includes(events.begin(), events.end(), g_subEvents.begin(), g_subEvents.end());
    return !ret;
}

void CupsMonitor::parseSubEvents(const string &events,  std::vector<std::string> &ret)
{
    QStringList eventList = QString::fromStdString(events).split("`", QString::SkipEmptyParts);
    foreach (QString var, eventList) {
        string event = var.toStdString().substr(1);
        event = event.replace(event.length() - 1, 1, "");
        ret.push_back(event);
    }
}

