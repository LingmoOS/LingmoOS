// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ZCUPSMONITOR_H
#define ZCUPSMONITOR_H

#include "ztaskinterface.h"
#include "config.h"

#include <QMutex>
#include <QSet>
#include <QDBusMessage>
#include <QTime>

#include <string>
#include <vector>

class CupsMonitor : public QThread
{
    Q_OBJECT

public:
    CupsMonitor(QObject *parent = nullptr);
    ~CupsMonitor() override;

    void initTranslations();
    int initSubscription();
    bool initWatcher();
    void clearSubscriptions();
    void stop();
    QString getJobMessage(int id);
    int getPrinterState(const QString &printer);

public slots:
    //dbus接口
    bool isJobPurged(int id);
    QString getJobNotify(const QMap<QString, QVariant> &job);
    QString getStateString(int iState);

protected:

    void run() override;
    int doWork();

    bool insertJobMessage(int id, int state, const QString &message);

    int cancelSubscription();
    int createSubscription();
    int getNotifications(int &notifysSize);
    int resetSubscription();

    // expired: timeout in millisecond
    //           0 - never expired
    //          -1 - server dependent
    int sendDesktopNotification(int replaceId, const QString &summary, const QString &body, int expired);

    bool isCompletedState(int state);


    void showJobsWindow();

    bool isNeedUpdateSubscription(std::vector<std::string> &events);
    void parseSubEvents(const std::string &events, std::vector<std::string> &ret);

    void writeJobLog(bool isSuccess, int jobId, QString strReason);

protected slots:
    void notificationInvoke(unsigned int, QString);
    void notificationClosed(unsigned int, unsigned int);
    void spoolerEvent(QDBusMessage);

signals:
    void signalJobStateChanged(int id, int state, const QString &message);
    void signalPrinterStateChanged(const QString &printer, int state, const QString &message);
    void signalPrinterDelete(const QString &printer);
    void signalPrinterAdd(const QString &printer);
    void signalShowTrayIcon(bool show);

private:
    QMap<int, QString> m_jobMessages;
    QMap<QString, int> m_printersState;
    QStringList m_stateStrings;
    QMutex m_mutex;

    int m_subId;
    int m_seqNumber;
    int m_jobId;

    bool m_bQuit;

    QSet<unsigned int> m_pendingNotification;
    QMap<int, QTime> m_processingJob;

};


#endif //ZCUPSMONITOR_H
