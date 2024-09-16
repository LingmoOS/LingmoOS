// Copyright (C) 2023 Dingyuan Zhang <lxz@mkacg.com>.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "singlewaylandhelper.h"

#include <QProcess>
#include <QDebug>
#include <QCoreApplication>
#include <qprocess.h>

SingleWaylandHelper::SingleWaylandHelper(QObject *parent)
    : QObject(parent)
    , m_wCompositorCrashCount(0)
    , m_maxCrashCountLimit(3)
{}

bool SingleWaylandHelper::start(const QString &compositor, const QString &cmd)
{
    auto args = QProcess::splitCommand(cmd);

    m_process = new QProcess(this);
    m_process->setProgram(compositor);
    m_process->setArguments(args);
    m_process->setProcessEnvironment([]{
        auto env = QProcessEnvironment::systemEnvironment();
        env.insert("LIBSEAT_BACKEND", "seatd");
        env.insert("DSG_APP_ID", "org.deepin.dde.treeland");
#if DEBUG
        env.insert("WAYLAND_DEBUG", "1");
#endif
        return env;
    }());

    connect(m_process, &QProcess::readyReadStandardError, this, [this] {
        qWarning() << m_process->readAllStandardError();
    });
    connect(m_process, &QProcess::readyReadStandardOutput, this, [this] {
        qInfo() << m_process->readAllStandardOutput();
    });
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            m_process, [this](int exitCode, QProcess::ExitStatus exitStatus) {
        qDebug() << "kwin_wayland exit with exitCode:" << exitCode << exitStatus;
        if (exitCode == 0) {
            qApp->quit();
            return;
        } else if (exitCode == 133) {
            m_wCompositorCrashCount = 0;
        } else {
            m_wCompositorCrashCount++;
        }

        if (m_wCompositorCrashCount > m_maxCrashCountLimit) {
            qApp->quit();
            return;
        }

        qWarning() << "WAYLAND Restart count: " << QByteArray::number(m_wCompositorCrashCount);

        // restart
        m_process->start();
    });

    m_process->start();
    if (!m_process->waitForStarted(10000)) {
        qWarning("Failed to start \"%s\": %s",
                 qPrintable(cmd),
                 qPrintable(m_process->errorString()));
        return false;
    }

    return true;
}
