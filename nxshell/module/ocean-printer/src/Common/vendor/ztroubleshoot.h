// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ZTROUBLESHOTT_H
#define ZTROUBLESHOTT_H

#include "ztaskinterface.h"

#include <QObject>
#include <QList>
#include <QEventLoop>
#include <QDBusMessage>

class TroubleShootJob : public QObject
{
    Q_OBJECT

public:
    TroubleShootJob(const QString &printerName, QObject *parent = nullptr)
        : QObject(parent)
    {
        m_printerName = printerName;
        m_bQuit = false;
    }

    virtual bool isPass() = 0;

    virtual QString getJobName() = 0;

    virtual void stop() { m_bQuit = true; }

    QString getMessage() { return m_strMessage; }

signals:
    void signalStateChanged(int state, const QString &message);

protected:
    QString m_printerName;
    QString m_strMessage;
    bool m_bQuit;
};

class PrinterTestJob : public TroubleShootJob
{
    Q_OBJECT

public:
    PrinterTestJob(const QString &printerName, QObject *parent = nullptr, bool bSync = true);
    ~PrinterTestJob() Q_DECL_OVERRIDE;

    bool isPass() Q_DECL_OVERRIDE;
    QString getJobName() Q_DECL_OVERRIDE;

    void stop() Q_DECL_OVERRIDE;

    bool findRunningJob();

protected slots:
    void slotJobStateChanged(const QDBusMessage &msg);

private:
    QEventLoop *m_eventLoop;
    int m_jobId;
    bool m_bSync;
};

class TroubleShoot : public TaskInterface
{
    Q_OBJECT

public:
    TroubleShoot(const QString &printerName, QObject *parent = nullptr);
    ~TroubleShoot() Q_DECL_OVERRIDE;

    int addJob(TroubleShootJob *job);

    QList<TroubleShootJob *> getJobs();

    void stop() Q_DECL_OVERRIDE;

protected:
    int doWork() Q_DECL_OVERRIDE;

private:
    QString m_printerName;

    QList<TroubleShootJob *> m_jobs;
};

#endif
