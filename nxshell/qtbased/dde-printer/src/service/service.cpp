// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "service.h"

#include <QCoreApplication>
#include <QTimer>
#include <QDebug>
#include <QStringList>

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QFileInfo>

static QString getMountNamespaceID(quint32 pid)
{
    QString path = QString("/proc/%1/ns/mnt").arg(pid);
    QFile file(path);

    if (!file.exists()) {
        qDebug() << "Mnt does not exist:" << path;
        return QString();
    }

    QString linkTarget = file.symLinkTarget(path);
    if (linkTarget.isEmpty()) {
        qDebug() << "Unable to get the symlink target for:" << path;
    }

    return linkTarget;
}

bool isMountNamespacesEqual(quint32 pid1, quint32 pid2) {
    QString ns1 = getMountNamespaceID(pid1);
    QString ns2 = getMountNamespaceID(pid2);
    qDebug() << "isMountNamespacesEqual: " << ns1 << ns2;
    if (ns1.isEmpty() || ns2.isEmpty()) {
        return false;
    }

    return ns1.mid(ns1.lastIndexOf('/') + 1) == ns2.mid(ns2.lastIndexOf('/') + 1);
}

static const QString getProcessPath(quint32 pid) {
    QString path;
    if (!isMountNamespacesEqual(pid, 1)) {
        return path;
    }

    QFileInfo file("/proc/" + QString::number(pid) + "/exe");
    if (file.exists() && file.isSymLink()) {
        path = file.symLinkTarget();
    }
    return path;
}

static QStringList g_autoStartList = QStringList() << "/opt/printer-drivers/com.pantum.pantum/config/"
                                            << "/opt/printer-drivers/com.lanxum-ga-series/config/"
                                            << "/opt/printer-drivers/com.hp.hplip/config/" ;

Service::Service(QObject *parent) : QObject(parent), QDBusContext()
{
    m_proc = new QProcess;

    connect(m_proc, &QProcess::stateChanged, [=](QProcess::ProcessState state) {
        if (state == QProcess::ProcessState::NotRunning) {
            QCoreApplication::exit();
        }
    });
    QTimer::singleShot(10000, QCoreApplication::instance(), &QCoreApplication::quit);
}

Service::~Service()
{
    delete m_proc;
    m_proc = nullptr;
}

void Service::LaunchAutoStart(const QString &filePath)
{
    QTimer::singleShot(100, QCoreApplication::instance(), &QCoreApplication::quit);

    if (filePath.isEmpty() || !g_autoStartList.contains(filePath)) return;

    if (!IsAuthorizePath()) {
        return;
    }
    QString scriptPath = filePath + "autostart.sh";
    QProcess::startDetached(scriptPath,  QStringList() << "-c");
    return;
}

int Service::CanonPrinterInstall(const QStringList &args)
{
    if (!IsAuthorizePath()) {
        return -11;
    }

    m_proc->start("/opt/deepin/dde-printer/printer-drivers/cndrvcups-capt/canonadd", args);
    m_proc->waitForFinished();
    return m_proc->exitCode();
}

int Service::CanonPrinterRemove(const QStringList &args)
{
    if (!IsAuthorizePath()) {
        return -11;
    }

    m_proc->start("/opt/deepin/dde-printer/printer-drivers/cndrvcups-capt/canonremove", args);
    m_proc->waitForFinished();
    return m_proc->exitCode();
}

bool Service::IsAuthorizePath()
{
    QDBusConnection conn = connection();
    QDBusMessage msg = message();
    QDBusReply<unsigned int> pid = conn.interface()->servicePid(msg.service());
    QString name = getProcessPath(pid);
    qDebug() << "path:" << pid << name;
    if (name == "/usr/bin/dde-printer" || name == "/usr/bin/dde-printer-helper") {
        return true;
    }
    return false;
}
