// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#include "insiderworker.h"

#include <QDir>
#include <QProcess>

namespace ocean {
namespace insider {
InsiderWorker::InsiderWorker(QObject *parent)
    : QObject(parent)
    , m_displayManager("lightdm")
{
    QMetaObject::invokeMethod(this, &InsiderWorker::checkEnabledDisplayManager, Qt::QueuedConnection);
}

InsiderWorker::~InsiderWorker() { }

QString InsiderWorker::displayManager() const
{
    return m_displayManager;
}

void InsiderWorker::setDisplayManager(const QString &id)
{
    if (m_displayManager == id) {
        return;
    }
    bool isNew = id == "treeland";
    switchDisplayManager(isNew);
    checkEnabledDisplayManager();
}

void InsiderWorker::checkEnabledDisplayManager()
{
    QProcess *process = new QProcess(this);
    process->setProgram("systemctl");
    process->setArguments(QStringList() << "is-enabled"
                                        << "lightdm.service");
    connect(process, &QProcess::finished, this, &InsiderWorker::onDisplayManagerFinished, Qt::QueuedConnection);
    process->start();
}

void InsiderWorker::onDisplayManagerFinished()
{
    QProcess *process = qobject_cast<QProcess *>(sender());
    if (!process) {
        return;
    }
    bool isLightdm = process->readAllStandardOutput().trimmed() == "enabled";
    QString displayManager = isLightdm ? "lightdm" : "treeland";
    if (m_displayManager != displayManager) {
        m_displayManager = displayManager;
        Q_EMIT displayManagerChanged(m_displayManager);
    }
}

void InsiderWorker::switchDisplayManager(bool isNew)
{
    QProcess process;
    process.setProgram("/usr/bin/pkexec");
    if (isNew) {
        // systemd service named ddm, not treeland
        process.setArguments(QStringList() << "systemctl"
                                           << "enable"
                                           << "ddm.service"
                                           << "-f");
    } else {
        process.setArguments(QStringList() << "systemctl"
                                           << "enable"
                                           << "lightdm.service"
                                           << "-f");
    }

    process.start();
    process.waitForFinished();
    qDebug() << "switchDisplayManager: " << process.readAll();
}

} // namespace insider
} // namespace ocean
