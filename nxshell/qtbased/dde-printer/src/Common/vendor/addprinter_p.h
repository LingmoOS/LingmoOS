// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ADDPRINTER_P_H
#define ADDPRINTER_P_H

#include "addprinter.h"
#include "zdevicemanager.h"

#include <QProcess>
#include <QDBusMessage>

class PrinterServerInterface;

typedef struct tagPackageInfo {
    QString toString() { return packageName + ": " + packageVer; }

    QString packageName;
    QString packageVer;
} TPackageInfo;

class InstallInterface : public QObject
{
    Q_OBJECT

public:
    explicit InstallInterface(QObject *parent = nullptr);

    void setPackages(const QList<TPackageInfo> &packages);
    void startInstallPackages(bool status = true);

    QString getErrorString();

    virtual void stop();

signals:
    void signalStatus(int);

protected slots:
    void propertyChanged(const QDBusMessage &msg);
    void updateSourceChanged(const QDBusMessage &msg);

protected:
    QList<TPackageInfo> m_packages;
    QStringList m_installPackages;
    bool m_bQuit;
    QString m_jobPath;
    QString m_updatePath;
    QString m_strType;
    QString m_strStatus;
    QString m_strUpdateType;
    QString m_strUpdateStatus;
    QString m_strErr;
};

class InstallDriver : public InstallInterface
{
    Q_OBJECT

public:
    InstallDriver(const QMap<QString, QVariant> &solution, QObject *parent = nullptr);

    void doWork();

    void stop();

protected slots:
    void slotServerDone(int iCode, const QByteArray &result);

private:
    QMap<QString, QVariant> m_solution;
};

class AddCanonCAPTPrinter : public AddPrinterTask
{
    Q_OBJECT

public:
    AddCanonCAPTPrinter(const TDeviceInfo &printer, const QMap<QString, QVariant> &solution, const QString &uri, QObject *parent = nullptr);

    void stop();

protected:
    int addPrinter();

protected slots:
    void slotProcessFinished(int iCode, QProcess::ExitStatus exitStatus);

private:
    QProcess m_proc;
};

class DefaultAddPrinter : public AddPrinterTask
{
    Q_OBJECT

public:
    DefaultAddPrinter(const TDeviceInfo &printer, const QMap<QString, QVariant> &solution, const QString &uri, QObject *parent = nullptr);

    void stop();

protected:
    int addPrinter();

protected slots:
    void slotProcessFinished(int iCode, QProcess::ExitStatus exitStatus);

private:
    QProcess m_proc;
};

class FixHplipBackend : public QObject
{
    Q_OBJECT

public:
    FixHplipBackend(QObject *parent = nullptr);

    int startFixed();
    QString getMatchHplipUri(const QString &strUri);

    void stop();

    QString getErrorString();

signals:
    void signalStatus(int);

protected slots:
    void slotDeviceStatus(int id, int status);
    void slotInstallStatus(int status);

private:
    RefreshDevicesByBackendTask *m_deviceTask;
    TBackendSchemes m_hplipBackend;

    InstallInterface *m_installer;

    QString m_strError;
};

#endif //ADDPRINTER_P_H
